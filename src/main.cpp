#include "core/auth/MicrosoftAuthService.h"
#include "core/download/MinecraftService.h"
#include "core/download/VanillaManifestService.h"
#include "core/java/JavaService.h"
#include "core/minecraft/MinecraftLauncher.h"
#include "core/modloader/ModloaderInstaller.h"
#include "core/profile/ProfileManager.h"
#include "core/settings/SettingsStore.h"
#include "web/WebHelper.h"
#include "core/install/InstallLock.h"
#include "core/log/LogBuffer.h"
#include "core/util/Paths.h"
#include "core/util/Paths.h"

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <system_error>
#include <thread>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
#endif

#ifndef WEB_DIR
    #define WEB_DIR "./web"
#endif

using namespace std;
using namespace hinge::core;

namespace {
    int findFreePort() {
        #ifdef _WIN32
            WSADATA wsa;
            if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) throw runtime_error("WSAStartup failed");

            SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == INVALID_SOCKET) {
                WSACleanup();
                throw runtime_error("socket() failed");
            }

            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            addr.sin_port = 0;

            if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
                int err = WSAGetLastError();
                closesocket(sock);
                WSACleanup();
                throw runtime_error("bind() failed: " + to_string(err));
            }

            int len = sizeof(addr);
            getsockname(sock, reinterpret_cast<sockaddr*>(&addr), &len);
            int port = ntohs(addr.sin_port);

            closesocket(sock);
            WSACleanup();
            return port;
        #else
            int fd = ::socket(AF_INET, SOCK_STREAM, 0);
            if (fd < 0) throw system_error(errno, generic_category(), "socket() failed");

            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            addr.sin_port = 0;

            if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
                int err = errno;
                ::close(fd);
                throw system_error(err, generic_category(), "bind() failed");
            }

            socklen_t len = sizeof(addr);
            getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len);
            int port = ntohs(addr.sin_port);

            ::close(fd);
            return port;
        #endif
    }

    nlohmann::json profileToJson(const data::Profile& p) {
        return {
            {"name",          p.nameOfProfile},
            {"minecraftName", p.minecraftName},
            {"uuid",          p.uuid},
            {"unlicensed",    p.unlicensed},
            {"path",          p.getActiveMinecraftPath().string()},
            {"memory",        p.getActiveMaxMemory()}
        };
    }

    struct MSAuthBridge : public auth::AuthCallback {
        profile::ProfileManager* profiles;

        explicit MSAuthBridge(profile::ProfileManager* p)
            : profiles(p) {}

        void onSuccess(string name, string uuid, string accessToken, string refreshToken) override {
            try {
                profiles->createMicrosoft(name, name, uuid, accessToken, refreshToken);
                cout << "[MS] Profile created: " << name << '\n';
            } catch (const exception& e) {
                cerr << "[MS] " << e.what() << '\n';
            }
        }

        void onFailure(string error) override {
            cerr << "[MS] Auth failed: " << error << '\n';
        }
    };
}

static void registerCommands(hinge::web::WebHelper& web,
                             settings::SettingsStore& settings,
                             profile::ProfileManager& profiles) {
    using nlohmann::json;

    auto runBackground = [](const string& label, function<void()> work) -> json {
        auto& lock = install::InstallLock::instance();
        if (!lock.tryAcquire(label)) {
            throw runtime_error("Уже идёт установка: " + lock.label());
        }

        thread([label, work = std::move(work)]() {
            auto& lk = install::InstallLock::instance();
            auto& lg = log::LogBuffer::instance();
            try {
                work();
            } catch (const exception& e) {
                lg.error("[" + label + "] " + string(e.what()));
            } catch (...) {
                lg.error("[" + label + "] неизвестная ошибка");
            }
            lk.release();
        }).detach();

        return {{"status", "started"}, {"label", label}};
    };

    web.commands().add("install.cancel")
        .title("Отменить установку")
        .category("Установка")
        .handler([&](const json&) -> json {
            install::InstallLock::instance().requestCancel();
            log::LogBuffer::instance().warn("Запрошена отмена установки");
            return {{"ok", true}};
        });

    web.commands().add("install.lock")
        .title("Состояние установки")
        .category("Установка")
        .handler([&](const json&) -> json {
            auto& lock = install::InstallLock::instance();
            return {
                {"busy", lock.isBusy()},
                {"label", lock.label()},
                {"cancelling", lock.cancelRequested()}
            };
        });

    web.commands().add("java.delete")
        .title("Удалить портативную Java")
        .category("Java")
        .intParam("version", "Версия Java (8, 17, 21)", 17)
        .handler([&](const json& p) -> json {
            int ver = p.value("version", 17);
            bool ok = java::JavaService::instance().removeJava(ver);
            if (ok) log::LogBuffer::instance().info("Java " + to_string(ver) + " удалена");
            return {{"ok", ok}, {"version", ver}};
        });

    web.commands().add("java.detect")
        .title("Определить требования к Java")
        .category("Java")
        .stringParam("version", "Версия Minecraft", true)
        .handler([&](const json& p) -> json {
            string v = p.value("version", "");
            int major = java::JavaService::resolveJavaVersion(0, v);
            return {{"requiredMajor", major}};
        });

    web.commands().add("java.installed")
        .title("Список скачанных Java")
        .category("Java")
        .handler([&](const json&) -> json {
            json arr = json::array();
            for (int v : {8, 17, 21}) {
                auto exe = java::JavaService::instance().findInstalled(v);
                if (!exe.empty()) {
                    arr.push_back({{"version", v}, {"path", exe.string()}});
                }
            }
            return {{"javas", arr}};
        });

    web.commands().add("java.download")
        .title("Скачать портативную Java")
        .category("Java")
        .intParam("version", "Версия Java (8, 17, 21)", 17)
        .handler([&](const json& p) -> json {
            int ver = p.value("version", 17);
            string label = "Java " + to_string(ver);

            auto& lock = install::InstallLock::instance();
            if (!lock.tryAcquire(label)) {
                throw runtime_error("Уже идёт установка: " + lock.label());
            }

            auto& log = log::LogBuffer::instance();
            try {
                log.info("Скачивание портативной Java " + to_string(ver));
                auto path = java::JavaService::instance().ensureJava(ver);
                log.info("Java " + to_string(ver) + " готова: " + path.string());
                lock.release();
                return {{"ok", true}, {"path", path.string()}, {"version", ver}};
            } catch (...) {
                lock.release();
                throw;
            }
        });

    web.commands().add("profile.setJava")
        .title("Назначить Java для пути")
        .category("Профили")
        .stringParam("path", "Путь профиля", true)
        .stringParam("java", "Путь к Java (пусто = Авто)", false, "")
        .handler([&](const json& p) -> json {
            auto* prof = profiles.active();
            if (!prof) throw runtime_error("Нет активного профиля");

            string path = p.value("path", "");
            string java = p.value("java", "");

            if (java.empty()) prof->javaPaths.erase(path);
            else              prof->javaPaths[path] = java;

            profiles.save(*prof);
            return {{"ok", true}};
        });

    web.commands().add("profile.list")
        .title("Список профилей").category("Профили")
        .handler([&](const json&) -> json {
            json arr = json::array();
            for (const auto& p : profiles.all()) arr.push_back(profileToJson(p));
            return {{"profiles", arr}};
        });

    web.commands().add("profile.current")
        .title("Активный профиль").category("Профили")
        .handler([&](const json&) -> json {
            if (auto* p = profiles.active()) return profileToJson(*p);
            return json{{"error", "Нет активного профиля"}};
        });

    web.commands().add("game.launch")
        .title("Запустить Minecraft")
        .category("Играть")
        .stringParam("version", "Версия Minecraft", true, "")
        .stringParam("modloader", "ID модлоадера (пусто = vanilla)", false, "")
        .handler([&](const json& p) -> json {
            auto& lock = install::InstallLock::instance();
            if (lock.isBusy())
                throw runtime_error("Идёт установка: " + lock.label() + ". Дождитесь завершения.");

            auto* active = profiles.active();
            if (!active) throw runtime_error("Нет активного профиля");

            string version = p.value("version", string());
            string modloader = p.value("modloader", string());
            if (version.empty()) throw runtime_error("version обязателен");

            auto gameDir = active->getActiveMinecraftPath();

            data::VersionDetails vanilla;
            try {
                vanilla = download::VanillaManifestService::loadLocal(gameDir, version);
            } catch (const exception&) {
                throw runtime_error("Версия " + version + " не установлена. Сначала нажми «Скачать».");
            }

            optional<data::VersionDetails> modDetails;
            if (!modloader.empty()) {
                try {
                    modDetails = download::VanillaManifestService::loadLocal(gameDir, modloader);
                } catch (const exception&) {
                    throw runtime_error("Модлоадер '" + modloader + "' не найден. Проверь, установлен ли он.");
                }
            }

            auto& svc = download::MinecraftService::instance();
            auto broken = svc.verifyVersion(vanilla, gameDir);
            if (modDetails) {
                auto brokenMod = svc.verifyVersion(*modDetails, gameDir);
                broken.insert(broken.end(), brokenMod.begin(), brokenMod.end());
            }

            if (!broken.empty()) {
                string msg = "Повреждённые файлы (" + to_string(broken.size()) + "): ";
                for (size_t i = 0; i < broken.size() && i < 3; ++i) msg += broken[i] + ", ";
                if (broken.size() > 3) msg += "... ";
                msg += "— нажми «Скачать» ещё раз.";
                throw runtime_error(msg);
            }

            data::Profile profileCopy = *active;

            log::LogBuffer::instance().info("Запуск Minecraft " + version +
                (modloader.empty() ? "" : " + " + modloader));

            thread([profileCopy = std::move(profileCopy), vanilla,
                    modDetails = std::move(modDetails), gameDir]() mutable {
                try {
                    minecraft::MinecraftLauncher launcher(gameDir, profileCopy);
                    auto t = launcher.launch(vanilla, modDetails ? &*modDetails : nullptr);
                    t.join();
                } catch (const exception& e) {
                    log::LogBuffer::instance().error(string("[launch] ") + e.what());
                }
            }).detach();

            return {
                {"status", "started"},
                {"version", version},
                {"modloader", modloader}
            };
        });

    web.commands().add("profile.create.offline")
        .title("Создать офлайн-профиль").category("Профили")
        .stringParam("name", "Имя профиля", true, "")
        .handler([&](const json& p) -> json {
            string name = p.value("name", "");
            if (name.empty()) throw runtime_error("name обязателен");
            auto& prof = profiles.createOffline(name);
            return profileToJson(prof);
        });

    web.commands().add("profile.create.ms")
        .title("Создать Microsoft-профиль")
        .description("Откроет браузер. Профиль появится в списке через ~5–30 сек.")
        .category("Профили")
        .boolParam("usePrism", "Использовать Prism client id", false)
        .handler([&](const json& p) -> json {
            bool usePrism = p.value("usePrism", false);
            auto* bridge = new MSAuthBridge(&profiles);

            thread([usePrism, bridge] {
                auth::MSAuthService::instance().authenticate(usePrism, *bridge);
                delete bridge;
            }).detach();

            return {{"status", "started"}};
        });

    web.commands().add("profile.pick")
        .title("Выбрать активный профиль").category("Профили")
        .stringParam("name", "Имя профиля", true, "")
        .handler([&](const json& p) -> json {
            profiles.setActive(p.value("name", ""));
            return profileToJson(*profiles.active());
        });

    web.commands().add("profile.delete")
        .title("Удалить профиль").category("Профили")
        .stringParam("name", "Имя профиля", true, "")
        .handler([&](const json& p) -> json {
            profiles.remove(p.value("name", ""));
            return {{"ok", true}};
        });

    web.commands().add("profile.paths")
        .title("Пути активного профиля").category("Профили")
        .handler([&](const json&) -> json {
            auto* p = profiles.active();
            if (!p) return json{{"paths", json::array()}};

            json arr = json::array();
            for (const auto& path : p->minecraftPaths) {
                json item = {
                    {"path", path},
                    {"selected", path == p->selectedMinecraftPath}
                };
                auto it = p->maxMemory.find(path);
                if (it != p->maxMemory.end()) item["memory"] = it->second;

                auto jt = p->javaPaths.find(path);
                if (jt != p->javaPaths.end()) item["java"] = jt->second;

                arr.push_back(std::move(item));
            }
            return {
                {"paths", arr},
                {"selected", p->selectedMinecraftPath},
                {"profile", p->nameOfProfile}
            };
        });

    web.commands().add("profile.addPath")
        .title("Добавить путь").category("Профили")
        .stringParam("path", "Путь к .minecraft", true, "")
        .intParam("memory", "Память (GB если <128, иначе MB)", 2)
        .handler([&](const json& p) -> json {
            auto* prof = profiles.active();
            if (!prof) throw runtime_error("Нет активного профиля");

            string path = p.value("path", "");
            int memory = p.value("memory", 2);
            if (path.empty()) throw runtime_error("path обязателен");

            if (find(prof->minecraftPaths.begin(), prof->minecraftPaths.end(), path) != prof->minecraftPaths.end()) {
                throw runtime_error("Такой путь уже добавлен");
            }

            prof->minecraftPaths.push_back(path);
            prof->maxMemory[path] = memory;
            if (prof->selectedMinecraftPath.empty()) prof->selectedMinecraftPath = path;

            profiles.save(*prof);
            return {{"ok", true}};
        });

    web.commands().add("profile.selectPath")
        .title("Выбрать путь").category("Профили")
        .stringParam("path", "Путь к .minecraft", true, "")
        .handler([&](const json& p) -> json {
            auto* prof = profiles.active();
            if (!prof) throw runtime_error("Нет активного профиля");

            string path = p.value("path", "");
            if (find(prof->minecraftPaths.begin(), prof->minecraftPaths.end(), path) == prof->minecraftPaths.end()) {
                throw runtime_error("Путь не найден");
            }

            prof->selectedMinecraftPath = path;
            profiles.save(*prof);
            return {{"ok", true}};
        });

    web.commands().add("profile.removePath")
        .title("Удалить путь").category("Профили")
        .stringParam("path", "Путь к .minecraft", true, "")
        .handler([&](const json& p) -> json {
            auto* prof = profiles.active();
            if (!prof) throw runtime_error("Нет активного профиля");

            string path = p.value("path", "");
            auto it = find(prof->minecraftPaths.begin(), prof->minecraftPaths.end(), path);
            if (it == prof->minecraftPaths.end()) throw runtime_error("Путь не найден");

            prof->minecraftPaths.erase(it);
            prof->maxMemory.erase(path);
            prof->javaPaths.erase(path);

            if (prof->selectedMinecraftPath == path) {
                prof->selectedMinecraftPath = prof->minecraftPaths.empty() ? "" : prof->minecraftPaths.front();
            }

            profiles.save(*prof);
            return {{"ok", true}};
        });

    web.commands().add("version.list")
        .title("Список версий Minecraft")
        .description("С фильтром по типу. Кешируется в памяти.")
        .category("Версии")
        .boolParam("release", "Релизы", true)
        .boolParam("snapshot", "Снапшоты", false)
        .boolParam("old_beta", "Старые беты", false)
        .boolParam("old_alpha", "Старые альфы", false)
        .handler([&](const json& p) -> json {
            bool release = p.value("release", true);
            bool snapshot = p.value("snapshot", false);
            bool oldBeta = p.value("old_beta", false);
            bool oldAlpha = p.value("old_alpha", false);

            auto manifest = download::VanillaManifestService::fetchManifest();

            json arr = json::array();
            for (const auto& v : manifest.versions) {
                if (v.type == "release"   && !release)  continue;
                if (v.type == "snapshot"  && !snapshot) continue;
                if (v.type == "old_beta"  && !oldBeta)  continue;
                if (v.type == "old_alpha" && !oldAlpha) continue;
                arr.push_back({{"id", v.id}, {"type", v.type}});
            }
            return {{"versions", arr}, {"count", arr.size()}};
        });

    web.commands().add("version.installed")
        .title("Установленные версии").category("Версии")
        .handler([&](const json&) -> json {
            auto* prof = profiles.active();
            if (!prof) return json{{"versions", json::array()}};

            auto versionsDir = prof->getActiveMinecraftPath() / "versions";
            json arr = json::array();

            error_code ec;
            if (filesystem::exists(versionsDir, ec)) {
                for (const auto& e : filesystem::directory_iterator(versionsDir, ec)) {
                    if (!e.is_directory()) continue;
                    auto name = e.path().filename().string();
                    auto jsonPath = e.path() / (name + ".json");
                    if (filesystem::exists(jsonPath)) arr.push_back(name);
                }
            }
            return {{"versions", arr}};
        });

    web.commands().add("install.vanilla")
        .title("Установить Minecraft")
        .description("Запускает скачивание в фоне. Прогресс — командой install.status.")
        .category("Установка")
        .stringParam("version", "ID версии, например 1.20.1", true, "1.20.1")
        .handler([&](const json& p) -> json {
            auto* active = profiles.active();
            if (!active) throw runtime_error("Нет активного профиля");

            string version = p.value("version", "");
            auto gameDir = active->getActiveMinecraftPath();

            return runBackground("Minecraft " + version, [version, gameDir]() {
                download::MinecraftService::instance().downloadVersion(version, gameDir);
            });
        });

    web.commands().add("install.status")
        .title("Прогресс установки")
        .category("Установка")
        .handler([&](const json&) -> json {
            return download::MinecraftService::instance().snapshot();
        });

    web.commands().add("install.threads")
        .title("Потоки скачивания")
        .category("Установка")
        .intParam("n", "Число потоков", 20)
        .handler([&](const json& p) -> json {
            int n = p.value("n", 20);
            download::MinecraftService::instance().setDownloadThreads(n);
            return {{"threads", n}};
        });

    web.commands().add("install.fabric")
        .title("Установить Fabric")
        .category("Установка")
        .stringParam("mc", "Версия Minecraft", true, "1.20.1")
        .stringParam("loader", "Версия Fabric Loader", true, "")
        .handler([&](const json& p) -> json {
            auto* active = profiles.active();
            if (!active) throw runtime_error("Нет активного профиля");

            auto mc = p.value("mc", string());
            auto loader = p.value("loader", string());
            auto dir = active->getActiveMinecraftPath();

            return runBackground("Fabric " + loader + " (" + mc + ")",
                                 [mc, loader, dir]() {
                modloader::ModloaderInstaller::installFabric(mc, loader, dir);
            });
        });

    web.commands().add("install.quilt")
        .title("Установить Quilt").category("Установка")
        .stringParam("mc", "Версия Minecraft", true, "1.20.1")
        .stringParam("loader", "Версия Quilt Loader", true, "")
        .handler([&](const json& p) -> json {
            auto* active = profiles.active();
            if (!active) throw runtime_error("Нет активного профиля");

            auto mc = p.value("mc", string());
            auto loader = p.value("loader", string());
            auto dir = active->getActiveMinecraftPath();

            return runBackground("Quilt " + loader + " (" + mc + ")",
                                 [mc, loader, dir]() {
                modloader::ModloaderInstaller::installQuilt(mc, loader, dir);
            });
        });

    web.commands().add("install.forge")
        .title("Установить Forge").category("Установка")
        .stringParam("mc", "Версия Minecraft", true, "1.20.1")
        .stringParam("loader", "Версия Forge", true, "")
        .handler([&](const json& p) -> json {
            auto* active = profiles.active();
            if (!active) throw runtime_error("Нет активного профиля");

            auto mc = p.value("mc", string());
            auto loader = p.value("loader", string());
            auto dir = active->getActiveMinecraftPath();

            return runBackground("Forge " + mc + "-" + loader,
                                 [mc, loader, dir]() {
                modloader::ModloaderInstaller::installForge(mc, loader, dir);
            });
        });

    web.commands().add("install.neoforge")
        .title("Установить NeoForge").category("Установка")
        .stringParam("loader", "Версия NeoForge (для 1.20.1 вида 1.20.1-47.1.0)", true, "")
        .handler([&](const json& p) -> json {
            auto* active = profiles.active();
            if (!active) throw runtime_error("Нет активного профиля");

            auto loader = p.value("loader", string());
            if (loader.empty()) throw runtime_error("loader обязателен");
            auto dir = active->getActiveMinecraftPath();

            return runBackground("NeoForge " + loader, [loader, dir]() {
                modloader::ModloaderInstaller::installNeoForge(loader, dir);
            });
        });

    web.commands().add("modloader.latest")
        .title("Последние версии лоадеров").category("Установка")
        .stringParam("type", "fabric|quilt|forge|neoforge", true, "fabric")
        .stringParam("mc", "Версия Minecraft", true, "1.20.1")
        .handler([&](const json& p) -> json {
            string type = p.value("type", string("fabric"));
            string mc = p.value("mc", string());

            auto httpGet = [](const string& url) -> string {
                auto scheme = url.find("://");
                if (scheme == string::npos) return "";
                auto slash = url.find('/', scheme + 3);
                string base = (slash == string::npos) ? url : url.substr(0, slash);
                string path = (slash == string::npos) ? "/" : url.substr(slash);

                httplib::Client cli(base);
                cli.enable_server_certificate_verification(false);
                cli.set_connection_timeout(10);
                cli.set_read_timeout(15);
                cli.set_follow_location(true);

                httplib::Headers headers = {
                    {"User-Agent", "curl/8.5.0"},
                    {"Accept", "*/*"}
                };

                auto res = cli.Get(path, headers);
                if (res && res->status == 200) return res->body;

                string cmd = "curl -sSL \"" + url + "\"";
                FILE* pipe = popen(cmd.c_str(), "r");
                if (pipe) {
                    string result;
                    char buf[1024];
                    while (fgets(buf, sizeof(buf), pipe)) result += buf;
                    int rc = pclose(pipe);
                    if (rc == 0 && !result.empty()) return result;
                }

                throw runtime_error("HTTP " + to_string(res ? res->status : -1) + ": " + url);
            };

            json arr = json::array();

            if (type == "fabric" || type == "quilt") {
                string url = (type == "fabric")
                    ? "https://meta.fabricmc.net/v2/versions/loader/" + mc
                    : "https://meta.quiltmc.org/v3/versions/loader/" + mc;

                auto data = nlohmann::json::parse(httpGet(url));
                int count = 0;
                for (const auto& item : data) {
                    if (count++ >= 15) break;
                    if (item.contains("loader") && item["loader"].contains("version")) {
                        arr.push_back({
                            {"version", item["loader"]["version"]},
                            {"stable",  item["loader"].value("stable", false)}
                        });
                    }
                }
            } else if (type == "forge") {
                string xml = httpGet("https://maven.minecraftforge.net/net/minecraftforge/forge/maven-metadata.xml");

                vector<string> all;
                string tag = "<version>";
                size_t pos = 0;
                while ((pos = xml.find(tag, pos)) != string::npos) {
                    size_t end = xml.find("</version>", pos);
                    if (end == string::npos) break;
                    string v = xml.substr(pos + tag.size(), end - pos - tag.size());
                    all.push_back(v);
                    pos = end + 1;
                }

                string prefix = mc + "-";
                int added = 0;
                for (auto it = all.rbegin(); it != all.rend() && added < 25; ++it) {
                    if (it->rfind(prefix, 0) == 0) {
                        string forgeBuild = it->substr(prefix.size());
                        arr.push_back({
                            {"version", forgeBuild},
                            {"stable", (added == 0)}
                        });
                        ++added;
                    }
                }

                if (arr.empty()) throw runtime_error("Нет версий Forge для " + mc);
            } else if (type == "neoforge") {
                bool legacy = (mc == "1.20.1");
                string url = legacy
                    ? "https://maven.neoforged.net/releases/net/neoforged/forge/maven-metadata.xml"
                    : "https://maven.neoforged.net/releases/net/neoforged/neoforge/maven-metadata.xml";

                string xml = httpGet(url);

                vector<string> all;
                string tag = "<version>";
                size_t pos = 0;
                while ((pos = xml.find(tag, pos)) != string::npos) {
                    size_t end = xml.find("</version>", pos);
                    if (end == string::npos) break;
                    string v = xml.substr(pos + tag.size(), end - pos - tag.size());
                    all.push_back(v);
                    pos = end + 1;
                }

                string prefix = legacy ? (mc + "-") : (mc.substr(2) + ".");

                int added = 0;
                for (auto it = all.rbegin(); it != all.rend() && added < 20; ++it) {
                    if (it->rfind(prefix, 0) == 0) {
                        arr.push_back({{"version", *it}, {"stable", true}});
                        ++added;
                    }
                }
                if (arr.empty()) throw runtime_error("Нет версий NeoForge для " + mc);
            } else {
                throw runtime_error("Неизвестный тип: " + type);
            }

            return {{"versions", arr}};
        });

    web.commands().add("modloader.installed")
        .title("Установленные модлоадеры").category("Версии")
        .handler([&](const json&) -> json {
            auto* prof = profiles.active();
            if (!prof) return json{{"modloaders", json::array()}};

            auto versionsDir = prof->getActiveMinecraftPath() / "versions";
            json arr = json::array();
            error_code ec;
            if (filesystem::exists(versionsDir, ec)) {
                for (const auto& e : filesystem::directory_iterator(versionsDir, ec)) {
                    if (!e.is_directory()) continue;
                    string name = e.path().filename().string();
                    string type;
                    if (name.rfind("fabric-loader-", 0) == 0) type = "fabric";
                    else if (name.rfind("quilt-loader-", 0) == 0) type = "quilt";
                    else if (name.rfind("neoforge-", 0) == 0) type = "neoforge";
                    else if (name.find("-forge-") != string::npos) type = "forge";
                    else continue;

                    auto jsonPath = e.path() / (name + ".json");
                    if (filesystem::exists(jsonPath)) {
                        arr.push_back({{"id", name}, {"type", type}});
                    }
                }
            }
            return {{"modloaders", arr}};
        });

    web.commands().add("settings.get")
        .title("Показать настройки")
        .category("Настройки")
        .handler([&](const json&) -> json {
            return settings.get();
        });

    web.commands().add("settings.update")
        .title("Изменить настройки")
        .category("Настройки")
        .intParam("port", "Порт (-1 = случайный)", 12345)
        .intParam("downloadThreads", "Число потоков скачивания", 8)
        .boolParam("closeWithSite", "Закрытие вкладки гасит лаунчер", true)
        .boolParam("autoOpenBrowser", "Открывать браузер при старте", true)
        .handler([&](const json& p) -> json {
            auto asBool = [](const json& j, bool def) -> bool {
                if (j.is_boolean()) return j.get<bool>();
                if (j.is_string()) {
                    string s = j.get<string>();
                    return s == "true" || s == "1" || s == "yes";
                }
                if (j.is_number()) return j.get<int>() != 0;
                return def;
            };

            settings.update([&](data::LauncherSettings& st) {
                st.port            = p.value("port", st.port);
                st.downloadThreads = p.value("downloadThreads", st.downloadThreads);
                st.closeWithSite   = asBool(p.value("closeWithSite",   json(st.closeWithSite)),   st.closeWithSite);
                st.autoOpenBrowser = asBool(p.value("autoOpenBrowser", json(st.autoOpenBrowser)), st.autoOpenBrowser);
            });

            auto& newCfg = settings.get();
            download::MinecraftService::instance().setDownloadThreads(newCfg.downloadThreads);

            return newCfg;
        });
}

int main() {
    using hinge::web::WebHelper;
    namespace fs = std::filesystem;

    fs::path root;
    if (const char* env = std::getenv("HINGE_DATA_DIR"); env && *env) {
        root = env;
    } else {
        root = hinge::core::util::getExecutableDir();
    }

    std::error_code ec;
    fs::create_directories(root, ec);
    auto probe = root / ".hinge-probe";
    {
        std::ofstream(probe) << "x";
        if (!fs::exists(probe)) {
            #ifdef _WIN32
                if (const char* a = std::getenv("APPDATA"); a && *a)
                    root = fs::path(a) / "Hinge";
            #else
                if (const char* h = std::getenv("HOME"); h && *h)
                    root = fs::path(h) / ".local" / "share" / "hinge";
            #endif
            fs::create_directories(root, ec);
        }
        fs::remove(probe, ec);
    }

    std::cout << "Data dir: " << root << '\n';

    settings::SettingsStore settings(root / "config.json");

    const auto& cfg = settings.get();
    const string bindAddr = "localhost";
    const int port = (cfg.port == -1) ? findFreePort() : cfg.port;

    std::cout << "Settings file: " << settings.path() << '\n';
    std::cout << "Binding to " << bindAddr << ":" << port << '\n';

    profile::ProfileManager profiles(root / "profiles", settings);
    profiles.load();

    if (profiles.all().empty()) {
        std::cout << "No profiles, creating default\n";
        profiles.createOffline("Player");
    }
    if (auto* p = profiles.active()) {
        std::cout << "Active profile: " << p->nameOfProfile << '\n';
    }

    atomic<bool> shouldExit{false};

    try {
        WebHelper web((root / "web").string());
        registerCommands(web, settings, profiles);

        web.post("/api/shutdown", [&](const nlohmann::json&) -> nlohmann::json {
            shouldExit = true;
            return {{"ok", true}};
        });

        const string url = web.start(bindAddr, port);
        cout << "Server: " << url << '\n';

        if (cfg.autoOpenBrowser) {
            this_thread::sleep_for(chrono::milliseconds(300));
            WebHelper::openInBrowser(url);
        }

        cout << "Running. Ctrl+C to stop.\n";
        while (!shouldExit) {
            this_thread::sleep_for(chrono::milliseconds(200));
        }
    } catch (const exception& e) {
        cerr << "Fatal: " << e.what() << '\n';
        return 1;
    }

    cout << "Bye.\n";
    return 0;
}
