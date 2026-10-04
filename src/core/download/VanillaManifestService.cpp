#include "core/download/VanillaManifestService.h"

#include <fstream>
#include <httplib.h>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>

using namespace std;

namespace hinge::core::download {
    namespace {
        constexpr auto kManifestHost = "piston-meta.mojang.com";
        constexpr auto kManifestPath = "/mc/game/version_manifest_v2.json";

        void splitUrl(const string& url, string& host, string& path) {
            auto scheme = url.find("://");
            if (scheme == string::npos) throw runtime_error("Bad URL: " + url);

            auto hostStart = scheme + 3;
            auto pathStart = url.find('/', hostStart);

            if (pathStart == string::npos) {
                host = url.substr(hostStart);
                path = "/";
            } else {
                host = url.substr(hostStart, pathStart - hostStart);
                path = url.substr(pathStart);
            }
        }
    }

    data::VersionDetails VanillaManifestService::fetchAndSave(const string& versionId,
                                                              const filesystem::path& gameDir) {
        httplib::SSLClient manifestCli(kManifestHost);
        manifestCli.enable_server_certificate_verification(false);
        manifestCli.set_connection_timeout(10);
        manifestCli.set_read_timeout(30);

        auto res = manifestCli.Get(kManifestPath);
        if (!res || res->status != 200) {
            throw runtime_error("Failed to fetch version manifest");
        }

        auto manifest = nlohmann::json::parse(res->body);
        string detailsUrl;

        for (const auto& entry : manifest["versions"]) {
            if (entry["id"] == versionId) {
                detailsUrl = entry["url"].get<string>();
                break;
            }
        }

        if (detailsUrl.empty()) {
            throw runtime_error("Version '" + versionId + "' not found in Mojang manifest");
        }

        string host, path;
        splitUrl(detailsUrl, host, path);

        httplib::SSLClient detailCli(host);
        detailCli.enable_server_certificate_verification(false);
        detailCli.set_connection_timeout(10);
        detailCli.set_read_timeout(30);

        auto detailRes = detailCli.Get(path);
        if (!detailRes || detailRes->status != 200) {
            throw runtime_error("Failed to fetch details for version " + versionId);
        }

        string detailJson = detailRes->body;

        filesystem::path versionFolder = gameDir / "versions" / versionId;
        filesystem::create_directories(versionFolder);
        ofstream(versionFolder / (versionId + ".json")) << detailJson;

        return nlohmann::json::parse(detailJson).get<data::VersionDetails>();
    }

    data::VersionDetails VanillaManifestService::loadLocal(const filesystem::path& gameDir,
                                                           const string& id) {
        filesystem::path jsonPath = gameDir / "versions" / id / (id + ".json");
        if (!filesystem::exists(jsonPath)) {
            throw runtime_error("Local config for '" + id + "' not found: " + jsonPath.string());
        }

        ifstream in(jsonPath);
        nlohmann::json j;
        in >> j;
        return j.get<data::VersionDetails>();
    }

    data::VersionManifest VanillaManifestService::fetchManifest() {
        static mutex mtx;
        static optional<data::VersionManifest> cache;

        lock_guard lk(mtx);
        if (cache.has_value()) return *cache;

        httplib::SSLClient cli(kManifestHost);
        cli.enable_server_certificate_verification(false);
        cli.set_connection_timeout(10);
        cli.set_read_timeout(30);

        auto res = cli.Get(kManifestPath);
        if (!res || res->status != 200) {
            throw runtime_error("Failed to fetch version manifest");
        }

        cache = nlohmann::json::parse(res->body).get<data::VersionManifest>();
        return *cache;
    }
}
