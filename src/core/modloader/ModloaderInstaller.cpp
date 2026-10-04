#include "core/modloader/ModloaderInstaller.h"
#include "core/download/MinecraftService.h"
#include "core/java/JavaService.h"
#include "core/install/InstallLock.h"
#include "core/log/LogBuffer.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <httplib.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>

#ifdef _WIN32
    #define popen  _popen
    #define pclose _pclose
#else
    #include <sys/wait.h>
#endif

using namespace std;

namespace hinge::core::modloader {
    namespace {
        string urlEncodeHost(const string& fullUrl, string& base, string& path) {
            auto scheme = fullUrl.find("://");
            if (scheme == string::npos) throw runtime_error("Bad URL: " + fullUrl);

            auto slash = fullUrl.find('/', scheme + 3);
            if (slash == string::npos) {
                base = fullUrl;
                path = "/";
            } else {
                base = fullUrl.substr(0, slash);
                path = fullUrl.substr(slash);
            }
            return base;
        }

        string httpGet(const string& url) {
            string base, path;
            urlEncodeHost(url, base, path);

            httplib::Client cli(base);
            cli.enable_server_certificate_verification(false);
            cli.set_connection_timeout(15);
            cli.set_read_timeout(60);
            cli.set_follow_location(true);

            httplib::Headers headers = {
                {"User-Agent", "curl/8.5.0"},
                {"Accept", "*/*"}
            };

            auto res = cli.Get(path, headers);
            if (res && res->status == 200) {
                return res->body;
            }

            string cmd = "curl -sSL \"" + url + "\"";
            FILE* pipe = popen(cmd.c_str(), "r");
            if (pipe) {
                string result;
                char buf[1024];
                while (fgets(buf, sizeof(buf), pipe)) {
                    result += buf;
                }
                int rc = pclose(pipe);
                if (rc == 0 && !result.empty()) return result;
            }

            throw runtime_error("GET failed (" + to_string(res ? res->status : -1) + "): " + url);
        }

        string mcFromNeoForge(const string& ver) {
            if (ver.rfind("1.20.1-", 0) == 0) return "1.20.1";

            auto firstDot = ver.find('.');
            if (firstDot == string::npos) return {};
            auto secondDot = ver.find('.', firstDot + 1);
            if (secondDot == string::npos) return {};

            string major = ver.substr(0, firstDot);
            string minor = ver.substr(firstDot + 1, secondDot - firstDot - 1);

            if (major == "20") return "1.20." + minor;
            if (major == "21") {
                if (minor == "0") return "1.21";
                return "1.21." + minor;
            }
            return {};
        }
    }

    string ModloaderInstaller::findJavaExecutable() {
        if (const char* home = getenv("JAVA_HOME")) {
            filesystem::path candidate = filesystem::path(home) / "bin" /
            #ifdef _WIN32
                "javaw.exe";
            #else
                "java";
            #endif
            if (filesystem::exists(candidate)) return candidate.string();
        }

        #ifdef _WIN32
            if (system("where java >nul 2>&1") == 0) return "javaw.exe";
        #else
            if (system("command -v java >/dev/null 2>&1") == 0) return "java";
        #endif

        throw runtime_error("Java not found. Set JAVA_HOME or install Java.");
    }

    int ModloaderInstaller::runProcess(const string& cmd) {
        auto& log = log::LogBuffer::instance();
        auto& lock = install::InstallLock::instance();

        string full = cmd + " 2>&1";
        FILE* pipe = popen(full.c_str(), "r");
        if (!pipe) throw runtime_error("popen failed: " + cmd);

        char buf[512];
        while (fgets(buf, sizeof(buf), pipe)) {
            string line(buf);
            while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) line.pop_back();
            if (!line.empty()) log.info("[installer] " + line);
            if (lock.cancelRequested()) {
                log.warn("Запрошена отмена, ожидание завершения инсталлятора");
            }
        }

        int rc = pclose(pipe);
        #ifdef _WIN32
            return rc;
        #else
            if (rc == -1) return -1;
            if (WIFEXITED(rc)) return WEXITSTATUS(rc);
            return rc;
        #endif
    }

    void ModloaderInstaller::installJsonModloader(const string& url,
                                                  const string& profileId,
                                                  const filesystem::path& gameDir,
                                                  const string& loaderName) {
        auto& svc = download::MinecraftService::instance();
        svc.setProgress(5, 0, 100, loaderName, "Загрузка профиля " + profileId);

        string json = httpGet(url);

        filesystem::path versionFolder = gameDir / "versions" / profileId;
        filesystem::create_directories(versionFolder);
        filesystem::path jsonPath = versionFolder / (profileId + ".json");

        ofstream(jsonPath) << json;
        cout << "Saved " << jsonPath.filename().string() << '\n';

        auto details = nlohmann::json::parse(json).get<data::VersionDetails>();

        svc.setProgress(20, 0, 100, loaderName, "Загрузка библиотек лоадера");
        svc.downloadVersion(details, gameDir);
    }

    void ModloaderInstaller::installFabric(const string& mcVersion,
                                           const string& loaderVersion,
                                           const filesystem::path& gameDir) {
        auto& svc = download::MinecraftService::instance();
        svc.setProgress(0, 0, 100, "Fabric", "Проверка ванильной версии " + mcVersion);
        svc.downloadVersion(mcVersion, gameDir);

        string profileId = "fabric-loader-" + loaderVersion + "-" + mcVersion;
        string url = "https://meta.fabricmc.net/v2/versions/loader/" +
                     mcVersion + "/" + loaderVersion + "/profile/json";

        cout << "Installing Fabric: " << profileId << '\n';
        installJsonModloader(url, profileId, gameDir, "Fabric");
    }

    void ModloaderInstaller::installQuilt(const string& mcVersion,
                                          const string& loaderVersion,
                                          const filesystem::path& gameDir) {
        auto& svc = download::MinecraftService::instance();
        svc.setProgress(0, 0, 100, "Quilt", "Проверка ванильной версии " + mcVersion);
        svc.downloadVersion(mcVersion, gameDir);

        string profileId = "quilt-loader-" + loaderVersion + "-" + mcVersion;
        string url = "https://meta.quiltmc.org/v3/versions/loader/" +
                     mcVersion + "/" + loaderVersion + "/profile/json";

        cout << "Installing Quilt: " << profileId << '\n';
        installJsonModloader(url, profileId, gameDir, "Quilt");
    }

    void ModloaderInstaller::installForge(const string& mcVersion,
                                          const string& forgeVersion,
                                          const filesystem::path& gameDir) {
        auto& svc = download::MinecraftService::instance();
        svc.setProgress(0, 0, 100, "Forge", "Проверка ванильной версии " + mcVersion);
        svc.downloadVersion(mcVersion, gameDir);

        filesystem::path generated = gameDir / "libraries" / "net" / "minecraft" / "client";
        error_code ec;
        if (filesystem::exists(generated, ec)) {
            filesystem::remove_all(generated, ec);
        }

        svc.setProgress(25, 25, 100, "Установка модлоадера с изолированной Java", "Подготовка среды");
        int javaVer = java::JavaService::resolveJavaVersion(0, mcVersion);
        string portableJava = java::JavaService::instance().ensureJava(javaVer).string();

        string installerUrl =
            "https://maven.minecraftforge.net/net/minecraftforge/forge/" +
            mcVersion + "-" + forgeVersion + "/forge-" +
            mcVersion + "-" + forgeVersion + "-installer.jar";

        downloadAndRunInstaller(installerUrl, gameDir,
                                "forge-" + mcVersion + "-" + forgeVersion, portableJava);
    }

    void ModloaderInstaller::installNeoForge(const string& loaderVersion,
                                             const filesystem::path& gameDir) {
        string mcVersion = mcFromNeoForge(loaderVersion);
        if (mcVersion.empty()) {
            throw runtime_error("Не могу определить версию Minecraft из NeoForge '" + loaderVersion + "'");
        }

        auto& svc = download::MinecraftService::instance();
        svc.setProgress(0, 0, 100, "NeoForge", "Проверка ванильной версии " + mcVersion);
        svc.downloadVersion(mcVersion, gameDir);

        if (mcVersion == "1.20.6") {
            string neoformRel = "net/neoforged/neoform/1.20.6-20240627.102356/neoform-1.20.6-20240627.102356.zip";
            string neoformUrl = "https://maven.neoforged.net/releases/" + neoformRel;
            filesystem::path dest = gameDir / "libraries" / neoformRel;

            svc.setProgress(25, 0, 100, "NeoForge", "Предзагрузка neoform.zip");
            filesystem::create_directories(dest.parent_path());
            if (!filesystem::exists(dest) || filesystem::file_size(dest) == 0) {
                string cmd = "curl -sSL -f -o \"" + dest.string() + "\" \"" + neoformUrl + "\"";
                system(cmd.c_str());
            }
        }

        svc.setProgress(25, 25, 100, "Установка модлоадера с изолированной Java", "Подготовка среды");
        int javaVer = java::JavaService::resolveJavaVersion(0, mcVersion);
        string portableJava = java::JavaService::instance().ensureJava(javaVer).string();

        string artifact = loaderVersion.starts_with("1.20.1") ? "forge" : "neoforge";
        string installerUrl =
            "https://maven.neoforged.net/releases/net/neoforged/" + artifact + "/" +
            loaderVersion + "/" + artifact + "-" + loaderVersion + "-installer.jar";

        downloadAndRunInstaller(installerUrl, gameDir, "neoforge-" + loaderVersion, portableJava);
    }

    void ModloaderInstaller::downloadAndRunInstaller(const string& url,
                                                    const filesystem::path& gameDir,
                                                    const string& installerName,
                                                    const string& javaPath) {
        auto& svc  = download::MinecraftService::instance();
        auto& log  = log::LogBuffer::instance();
        auto& lock = install::InstallLock::instance();

        filesystem::path launcherProfiles = gameDir / "launcher_profiles.json";
        if (!filesystem::exists(launcherProfiles)) {
            filesystem::create_directories(gameDir);
            ofstream(launcherProfiles) << R"({"profiles":{}})";
        }

        filesystem::path tempDir     = filesystem::temp_directory_path();
        filesystem::path installerJar = tempDir / (installerName + "-installer.jar");

        if (lock.cancelRequested()) throw runtime_error("Установка отменена");

        svc.setProgress(40, 0, 100, installerName, "Скачивание инсталлятора");
        svc.beginPhase(installerName + " — скачивание инсталлятора", 0);
        log.info("Скачивание инсталлятора " + installerName + ": " + url);

        {
            string base, path;
            urlEncodeHost(url, base, path);

            httplib::Client cli(base);
            cli.enable_server_certificate_verification(false);
            cli.set_connection_timeout(30);
            cli.set_read_timeout(600);
            cli.set_follow_location(true);

            httplib::Headers headers = {
                {"User-Agent", "curl/8.5.0"},
                {"Accept", "*/*"}
            };

            ofstream out;
            int status = 0;
            int64_t total = 0;
            int64_t got = 0;
            bool aborted = false;

            auto res = cli.Get(path, headers,
                [&](const httplib::Response& r) {
                    status = r.status;
                    if (r.status != 200) return true;

                    auto cl = r.get_header_value("Content-Length");
                    if (!cl.empty()) {
                        try { total = stoll(cl); } catch (...) { total = 0; }
                    }
                    if (total > 0)
                        svc.beginPhase(installerName + " — скачивание инсталлятора", total);
                    out.open(installerJar, ios::binary);
                    return true;
                },
                [&](const char* data, size_t len) {
                    if (!out.is_open()) return false;
                    if (lock.cancelRequested()) { aborted = true; return false; }

                    out.write(data, static_cast<streamsize>(len));
                    got += static_cast<int64_t>(len);
                    svc.addBytes(static_cast<int64_t>(len));

                    if (total > 0) {
                        svc.setPhaseDetail(installerName + " — скачивание инсталлятора",
                            to_string(got / 1024) + " KB / " + to_string(total / 1024) + " KB");
                    }
                    return true;
                });

            if (out.is_open()) out.close();

            if (aborted) {
                error_code ec;
                filesystem::remove(installerJar, ec);
                throw runtime_error("Установка отменена");
            }

            bool ok = res && status == 200 &&
                    filesystem::exists(installerJar) &&
                    filesystem::file_size(installerJar) > 0;

            if (!ok) {
                log.warn("httplib не смог скачать инсталлятор, повтор через curl");
                string cmd = "curl -sSL -f -o \"" + installerJar.string() + "\" \"" + url + "\"";
                if (system(cmd.c_str()) != 0 || !filesystem::exists(installerJar)) {
                    throw runtime_error("Не удалось скачать инсталлятор: " + url);
                }
            }

            error_code ec;
            auto sz = filesystem::file_size(installerJar, ec);
            log.info("Инсталлятор скачан: " + installerName + "-installer.jar, " +
                    to_string(ec ? 0 : sz / 1024) + " KB");
        }

        if (lock.cancelRequested()) throw runtime_error("Установка отменена");

        svc.setProgress(70, 0, 100, installerName, "Запуск инсталлятора");
        svc.beginPhase(installerName + " — Java-процессы (см. консоль)", 0);
        svc.setPhaseDetail(installerName + " — Java-процессы", "вывод инсталлятора в консоли");

        string java = javaPath.empty() ? findJavaExecutable() : javaPath;
        log.info("Используемая Java: " + java);

        string envPrefix;
        #ifndef _WIN32
            filesystem::path customZlib = gameDir / "bin" / "libz.so.1";
            if (filesystem::exists(customZlib)) {
                envPrefix = "LD_PRELOAD=\"" + customZlib.string() + "\" ";
            }
        #endif

        string jvmFlags = "-Dforgewrapper.skipHashCheck=true "
                        "-Dnet.minecraftforge.installer.skipHashCheck=true "
                        "-Djava.net.preferIPv4Stack=true ";

        string cmd = envPrefix + "\"" + java + "\" " + jvmFlags + "-jar \"" +
                    installerJar.string() + "\" --installClient \"" +
                    filesystem::absolute(gameDir).string() + "\"";

        log.info("Запуск " + installerName + " инсталлятора");
        int exitCode = runProcess(cmd);

        error_code ec;
        filesystem::remove(installerJar, ec);
        filesystem::remove(tempDir / (installerName + "-installer.jar.log"), ec);
        filesystem::remove(gameDir / (installerName + "-installer.jar.log"), ec);

        if (exitCode != 0) {
            if (lock.cancelRequested())
                throw runtime_error("Установка отменена пользователем");
            throw runtime_error(installerName + " инсталлятор завершился с кодом " +
                                to_string(exitCode));
        }

        log.info(installerName + " успешно установлен");
        svc.setProgress(100, 100, 100, "Готово", "Done");
    }
}
