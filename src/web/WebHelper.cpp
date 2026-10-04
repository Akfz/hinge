#include "web/WebHelper.h"
#include "web/DefaultWeb.h"
#include "core/install/InstallLock.h"
#include "core/log/LogBuffer.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

using namespace std;

namespace hinge::web {
    namespace {
        void ensureDefaultWebFiles(const std::string& dir) {
            std::error_code ec;
            std::filesystem::create_directories(dir, ec);
            if (ec) {
                throw std::runtime_error(
                    "Cannot create web dir '" + dir + "': " + ec.message() +
                    "\nSet HINGE_DATA_DIR to a writable location.");
            }

            auto writeIfMissing = [&](const char* name, const char* content) {
                auto path = std::filesystem::path(dir) / name;
                if (!std::filesystem::exists(path)) {
                    std::ofstream(path) << content;
                    std::cout << "Created def. file " << path << '\n';
                }
            };

            writeIfMissing("index.html", DefaultWeb::kIndexHtml);
            writeIfMissing("style.css",  DefaultWeb::kStyleCss);
            writeIfMissing("hinge.js",   DefaultWeb::kHingeJs);
            writeIfMissing("app.js",     DefaultWeb::kAppJs);
            writeIfMissing("live.js",    DefaultWeb::kLiveJs);
        }
    }

    WebHelper::WebHelper(string webDir)
        : webDir_(std::move(webDir)) {
        ensureDefaultWebFiles(webDir_);

        server_.set_post_routing_handler([](const httplib::Request&, httplib::Response& res) {
            res.set_header("Cache-Control", "no-cache, no-store, must-revalidate");
            res.set_header("Pragma", "no-cache");
            res.set_header("Expires", "0");
        });

        server_.Get("/api/web-version", [this](const httplib::Request&, httplib::Response& res) {
            res.set_content(
                nlohmann::json{{"version", buildVersion_.load()}}.dump(),
                "application/json"
            );
        });

        server_.Get("/api/commands", [this](const httplib::Request&, httplib::Response& res) {
            res.set_content(commands_.listJson().dump(), "application/json");
        });

        server_.Post(R"(/api/commands/([A-Za-z0-9_.]+))",
            [this](const httplib::Request& req, httplib::Response& res){
            string id = req.matches[1].str();
            nlohmann::json params = nlohmann::json::object();
            if (!req.body.empty()) {
                try { params = nlohmann::json::parse(req.body); } catch (...) {}
            }
            res.set_content(commands_.invoke(id, params).dump(), "application/json");
        });

        server_.Get("/api/logs", [](const httplib::Request& req, httplib::Response& res) {
            uint64_t since = 0;
            if (req.has_param("since")) {
                try { since = stoull(req.get_param_value("since")); } catch (...) {}
            }
            auto entries = core::log::LogBuffer::instance().since(since);
            nlohmann::json arr = nlohmann::json::array();
            for (const auto& e : entries) {
                const char* lvl = "info";
                if (e.level == core::log::Level::Warn)  lvl = "warn";
                if (e.level == core::log::Level::Error) lvl = "error";
                arr.push_back({
                    {"seq", e.seq},
                    {"level", lvl},
                    {"message", e.message},
                    {"ts", e.timestamp}
                });
            }
            res.set_content(nlohmann::json{
                {"entries", arr},
                {"lastSeq", core::log::LogBuffer::instance().lastSeq()}
            }.dump(), "application/json");
        });

        server_.Post("/api/logs/clear", [](const httplib::Request&, httplib::Response& res) {
            core::log::LogBuffer::instance().clear();
            res.set_content("{\"ok\":true}", "application/json");
        });

        server_.Post("/api/install/cancel", [](const httplib::Request&, httplib::Response& res) {
            core::install::InstallLock::instance().requestCancel();
            res.set_content("{\"ok\":true}", "application/json");
        });

        server_.set_mount_point("/", webDir_);
    }

    WebHelper::~WebHelper() { stop(); }

    void WebHelper::get(const string& path, JsonHandler handler) {
        server_.Get(path, [handler](const httplib::Request& req, httplib::Response& res) {
            try {
                nlohmann::json reqJson = nlohmann::json::object();
                for (const auto& [k, v] : req.params) reqJson[k] = v;
                if (!req.body.empty()) {
                    try { reqJson = nlohmann::json::parse(req.body); } catch (...) {}
                }

                auto respJson = handler(reqJson);
                res.set_content(respJson.dump(), "application/json");
            } catch (const exception& e) {
                res.status = 500;
                res.set_content(
                    nlohmann::json{{"error", e.what()}}.dump(),
                    "application/json"
                );
            }
        });
    }

    void WebHelper::post(const string& path, JsonHandler handler) {
        server_.Post(path, [handler](const httplib::Request& req, httplib::Response& res) {
            try {
                nlohmann::json reqJson = nlohmann::json::object();
                if (!req.body.empty()) reqJson = nlohmann::json::parse(req.body);

                auto respJson = handler(reqJson);
                res.set_content(respJson.dump(), "application/json");
            } catch (const exception& e) {
                res.status = 500;
                res.set_content(
                    nlohmann::json{{"error", e.what()}}.dump(),
                    "application/json"
                );
            }
        });
    }

    string WebHelper::start(const string& host, int port) {
        host_ = host;
        port_ = port;
        url_  = "http://" + host + ":" + to_string(port);

        watcher_ = thread([this] { watchLoop(); });
        serverThread_ = thread([this, host, port] {
            server_.listen(host, port);
        });

        return url_;
    }

    void WebHelper::stop() {
        stopWatcher_ = true;
        if (watcher_.joinable()) watcher_.join();

        server_.stop();
        if (serverThread_.joinable()) serverThread_.join();
    }

    void WebHelper::watchLoop() {
        auto last = computeStamp();

        while (!stopWatcher_) {
            this_thread::sleep_for(chrono::milliseconds(500));

            auto current = computeStamp();
            if (current != last) {
                last = current;
                buildVersion_.fetch_add(1);
                cout << "Web changed, version = "
                        << buildVersion_.load() << endl;
            }
        }
    }

    uint64_t WebHelper::computeStamp() const {
        uint64_t stamp = 0;
        error_code ec;

        for (auto it = fs::recursive_directory_iterator(webDir_, ec);
            !ec && it != fs::recursive_directory_iterator();
            it.increment(ec)) {

            if (!it->is_regular_file(ec)) continue;

            auto mtime = fs::last_write_time(it->path(), ec);
            if (ec) continue;

            auto size = fs::file_size(it->path(), ec);
            if (ec) continue;

            uint64_t h = static_cast<uint64_t>(mtime.time_since_epoch().count())
                            ^ static_cast<uint64_t>(size);
            stamp ^= hash<uint64_t>{}(h);
        }
        return stamp;
    }

    void WebHelper::openInBrowser(const string& url) {
        #ifdef _WIN32
            string cmd = "start \"\" \"" + url + "\"";
        #elif __APPLE__
            string cmd = "open \"" + url + "\"";
        #else
            string cmd = "xdg-open \"" + url + "\"";
        #endif
            system(cmd.c_str());
    }
}
