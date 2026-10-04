#include "core/minecraft/MinecraftLauncher.h"
#include "core/java/JavaService.h"
#include "core/util/Maven.h"
#include "core/log/LogBuffer.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <thread>

#ifdef _WIN32
    #define popen  _popen
    #define pclose _pclose
#endif

using namespace std;

namespace hinge::core::minecraft {
    string MinecraftLauncher::currentOs() {
        #ifdef _WIN32
            return "windows";
        #elif __APPLE__
            return "osx";
        #else
            return "linux";
        #endif
    }

    string MinecraftLauncher::shellQuote(const string& s) {
        #ifdef _WIN32
            string out = "\"";
            for (char c : s) {
                if (c == '"') out += '\\';
                out += c;
            }
            out += "\"";
            return out;
        #else
            string out = "'";
            for (char c : s) {
                if (c == '\'') out += "'\\''";
                else out += c;
            }
            out += "'";
            return out;
        #endif
    }

    string MinecraftLauncher::findJavaExecutable() {
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
            return "javaw.exe";
        #else
            return "java";
        #endif
    }

    MinecraftLauncher::MinecraftLauncher(filesystem::path minecraftDir, data::Profile& profile)
        : pathToMinecraft_(std::move(minecraftDir))
        , loadFrom_(&profile) {
        if (auto java = profile.getActiveJavaPath()) {
            pathToJava_ = *java;
        } else {
            pathToJava_ = findJavaExecutable();
        }
    }

    bool MinecraftLauncher::isArgumentAllowed(const nlohmann::json& argObj) {
        if (!argObj.is_object()) return true;
        if (!argObj.contains("rules")) return true;

        const auto& rules = argObj["rules"];
        if (!rules.is_array() || rules.empty()) return true;

        bool allowed = false;
        for (const auto& rule : rules) {
            if (!rule.is_object()) continue;

            bool matches = true;
            if (rule.contains("os") && rule["os"].is_object()) {
                const auto& os = rule["os"];
                if (os.contains("name")) {
                    string required = os["name"].get<string>();
                    if (required != currentOs()) matches = false;
                }
            }

            if (matches) {
                string action = rule.value("action", string());
                allowed = (action == "allow");
            }
        }
        return allowed;
    }

    bool MinecraftLauncher::isLibraryAllowed(const data::Library& lib) {
        if (lib.rules.empty()) return true;

        bool allowed = false;
        for (const auto& rule : lib.rules) {
            bool matches = true;

            if (rule.os.is_object() && rule.os.contains("name")) {
                string required = rule.os["name"].get<string>();
                if (required != currentOs()) matches = false;
            }

            if (matches) {
                allowed = (rule.action == "allow");
            }
        }
        return allowed;
    }

    string MinecraftLauncher::getLibraryPath(const data::Library& lib) {
        if (!lib.downloads.artifact.path.empty()) return lib.downloads.artifact.path;
        if (!lib.name.empty()) return util::mavenToPath(lib.name);
        return {};
    }

    void MinecraftLauncher::collectLibraries(const data::VersionDetails& details,
                                             vector<filesystem::path>& out) {
        for (const auto& lib : details.libraries) {
            if (!isLibraryAllowed(lib)) continue;

            string rel = getLibraryPath(lib);
            if (rel.empty()) continue;

            filesystem::path full = (pathToMinecraft_ / "libraries" / rel).lexically_normal();
            if (find(out.begin(), out.end(), full) == out.end()) {
                out.push_back(full);
            }
        }
    }

    bool MinecraftLauncher::isQuickPlayArg(const string& arg) {
        return arg == "--quickPlayPath"
            || arg == "--quickPlaySingleplayer"
            || arg == "--quickPlayMultiplayer"
            || arg == "--quickPlayRealms";
    }

    string MinecraftLauncher::substituteJvm(const string& arg,
                                                 const data::VersionDetails& vanilla,
                                                 const string& classpath) {
        static const string pathSep =
        #ifdef _WIN32
            ";";
        #else
            ":";
        #endif

        string out = arg;
        auto rep = [&](const string& from, const string& to) {
            size_t pos = 0;
            while ((pos = out.find(from, pos)) != string::npos) {
                out.replace(pos, from.size(), to);
                pos += to.size();
            }
        };

        rep("${natives_directory}", (pathToMinecraft_ / "bin" / "natives").string());
        rep("${launcher_name}", "hinge");
        rep("${launcher_version}", "1.0");
        rep("${classpath}", classpath);
        rep("${library_directory}", (pathToMinecraft_ / "libraries").string());
        rep("${classpath_separator}", pathSep);
        rep("${version_name}", vanilla.id);

        return out;
    }

    void MinecraftLauncher::ensureEssentialJvmArgs(vector<string>& jvmArgs) {
        static const char* essential[] = {
            "java.base/java.lang.invoke=ALL-UNNAMED",
            "java.base/java.io=ALL-UNNAMED",
            "java.base/java.lang.reflect=ALL-UNNAMED",
            "java.base/java.util.jar=ALL-UNNAMED",
            "java.base/java.util=ALL-UNNAMED",
            "java.base/java.lang=ALL-UNNAMED",
            "java.base/java.net=ALL-UNNAMED",
            "java.base/java.nio=ALL-UNNAMED",
        };

        for (auto* o : essential) {
            string full = string("--add-opens=") + o;
            if (find(jvmArgs.begin(), jvmArgs.end(), full) == jvmArgs.end()) {
                jvmArgs.push_back(full);
            }
        }
    }

    void MinecraftLauncher::parseJvmArgs(const vector<nlohmann::json>& raw,
                                         vector<string>& out,
                                         const data::VersionDetails& vanilla,
                                         const string& classpath) {
        for (const auto& item : raw) {
            if (item.is_string()) {
                string s = item.get<string>();
                if (s == "-cp" || s == "${classpath}") continue;
                out.push_back(substituteJvm(s, vanilla, classpath));
            } else if (item.is_object() && isArgumentAllowed(item)) {
                if (!item.contains("value")) continue;
                const auto& val = item["value"];

                if (val.is_string()) {
                    string s = val.get<string>();
                    if (s == "-cp" || s == "${classpath}") continue;
                    out.push_back(substituteJvm(s, vanilla, classpath));
                } else if (val.is_array()) {
                    for (const auto& v : val) {
                        if (!v.is_string()) continue;
                        string s = v.get<string>();
                        if (s == "-cp" || s == "${classpath}") continue;
                        out.push_back(substituteJvm(s, vanilla, classpath));
                    }
                }
            }
        }
    }

    vector<string> MinecraftLauncher::buildJvmArgs(const data::VersionDetails& active,
                                                   const data::VersionDetails& vanilla,
                                                   const string& classpath) {
        vector<string> jvm;

        if (!active.arguments.jvm.empty()) {
            parseJvmArgs(active.arguments.jvm, jvm, vanilla, classpath);
        } else if (!vanilla.arguments.jvm.empty()) {
            parseJvmArgs(vanilla.arguments.jvm, jvm, vanilla, classpath);
        }

        ensureEssentialJvmArgs(jvm);
        return jvm;
    }

    string MinecraftLauncher::substituteGame(const string& arg,
                                                  const data::VersionDetails& active,
                                                  const string& assetIndexId) {
        string username = loadFrom_->minecraftName.empty() ? "Player" : loadFrom_->minecraftName;
        string uuid = loadFrom_->uuid.empty() ? "0" : loadFrom_->uuid;
        string token = loadFrom_->accessToken.empty() ? "0" : loadFrom_->accessToken;

        string out = arg;
        auto rep = [&](const string& from, const string& to) {
            size_t pos = 0;
            while ((pos = out.find(from, pos)) != string::npos) {
                out.replace(pos, from.size(), to);
                pos += to.size();
            }
        };

        rep("${auth_player_name}", username);
        rep("${version_name}", active.id);
        rep("${game_directory}", loadFrom_->getActiveMinecraftPath().string());
        rep("${assets_root}", (pathToMinecraft_ / "assets").string());
        rep("${assets_index_name}", assetIndexId);
        rep("${auth_uuid}", uuid);
        rep("${auth_access_token}", token);
        rep("${user_properties}", "{}");
        rep("${clientid}", "clientId");
        rep("${auth_xuid}", "0");
        rep("${resolution_width}", "854");
        rep("${resolution_height}", "480");
        rep("${user_type}", "msa");
        rep("${version_type}", "release");

        return out;
    }

    void MinecraftLauncher::parseGameArgs(const vector<nlohmann::json>& raw,
                                          vector<string>& out,
                                          const data::VersionDetails& active,
                                          const string& assetIndexId) {
        bool skipNext = false;

        for (const auto& item : raw) {
            if (skipNext) {
                skipNext = false;
                continue;
            }

            if (item.is_string()) {
                string s = item.get<string>();
                if (isQuickPlayArg(s)) {
                    skipNext = true;
                    continue;
                }
                if (s == "--demo") continue;
                out.push_back(substituteGame(s, active, assetIndexId));
            } else if (item.is_object() && isArgumentAllowed(item)) {
                if (!item.contains("value")) continue;
                const auto& val = item["value"];

                auto handle = [&](const string& s) {
                    if (isQuickPlayArg(s)) return false;
                    if (s == "--demo") return false;
                    out.push_back(substituteGame(s, active, assetIndexId));
                    return true;
                };

                if (val.is_string()) {
                    handle(val.get<string>());
                } else if (val.is_array()) {
                    bool skipNextLocal = false;
                    for (const auto& v : val) {
                        if (skipNextLocal) {
                            skipNextLocal = false;
                            continue;
                        }
                        if (!v.is_string()) continue;
                        string s = v.get<string>();
                        if (isQuickPlayArg(s)) {
                            skipNextLocal = true;
                            continue;
                        }
                        handle(s);
                    }
                }
            }
        }
    }

    vector<string> MinecraftLauncher::buildGameArgs(const data::VersionDetails& active,
                                                    const data::VersionDetails& vanilla,
                                                    const string& assetIndexId) {
        vector<string> gameArgs;

        if (!vanilla.arguments.game.empty()) {
            parseGameArgs(vanilla.arguments.game, gameArgs, active, assetIndexId);
        } else if (!vanilla.minecraftArguments.empty()) {
            stringstream ss(vanilla.minecraftArguments);
            string tok;
            while (ss >> tok) gameArgs.push_back(substituteGame(tok, active, assetIndexId));
        }

        if (&active != &vanilla) {
            if (!active.arguments.game.empty()) {
                parseGameArgs(active.arguments.game, gameArgs, active, assetIndexId);
            } else if (!active.minecraftArguments.empty()) {
                stringstream ss(active.minecraftArguments);
                string tok;
                while (ss >> tok) gameArgs.push_back(substituteGame(tok, active, assetIndexId));
            }
        }

        return gameArgs;
    }

    vector<string> MinecraftLauncher::buildCommand(const data::VersionDetails& active,
                                                   const data::VersionDetails& vanilla) {
        if (!loadFrom_->getActiveJavaPath().has_value()) {
            int req = active.javaVersion.majorVersion;
            int neededJava = java::JavaService::resolveJavaVersion(req, active.id);
            pathToJava_ = java::JavaService::instance().ensureJava(neededJava);
        }

        vector<filesystem::path> libs;
        collectLibraries(vanilla, libs);
        if (&active != &vanilla) collectLibraries(active, libs);

        const char sep =
        #ifdef _WIN32
            ';';
        #else
            ':';
        #endif

        string classpath;
        for (const auto& p : libs) {
            classpath += p.string();
            classpath += sep;
        }
        classpath += (pathToMinecraft_ / "versions" / vanilla.id / (vanilla.id + ".jar")).string();

        string assetIndexId = vanilla.assetIndex.id;
        if (!active.assetIndex.id.empty()) assetIndexId = active.assetIndex.id;

        vector<string> args;
        args.push_back(pathToJava_.string());

        int maxMem = loadFrom_->getActiveMaxMemory();
        args.push_back(maxMem < 128 ? ("-Xmx" + to_string(maxMem) + "G")
                                    : ("-Xmx" + to_string(maxMem) + "m"));

        auto jvm = buildJvmArgs(active, vanilla, classpath);
        args.insert(args.end(), jvm.begin(), jvm.end());
        filesystem::path nativesDir = pathToMinecraft_ / "bin" / "natives";
        args.push_back("-Djava.library.path=" + nativesDir.string());
        args.push_back("-cp");
        args.push_back(classpath);
        args.push_back(active.mainClass);

        auto gameArgs = buildGameArgs(active, vanilla, assetIndexId);
        args.insert(args.end(), gameArgs.begin(), gameArgs.end());

        return args;
    }

   void MinecraftLauncher::runProcess(const vector<string>& args,
                                   const filesystem::path& cwd,
                                   const unordered_map<string, string>& env) {
        auto& log = log::LogBuffer::instance();

        string cmd;

        #ifdef _WIN32
            cmd = "cd /D " + shellQuote(cwd.string()) + " && ";
            for (const auto& [k, v] : env) cmd += "set " + k + "=" + shellQuote(v) + " && ";
        #else
            cmd = "cd " + shellQuote(cwd.string()) + " && ";
            for (const auto& [k, v] : env) cmd += k + "=" + shellQuote(v) + " ";
        #endif

        cmd += shellQuote(args[0]);
        for (size_t i = 1; i < args.size(); ++i)
            cmd += " " + shellQuote(args[i]);
        cmd += " 2>&1";

        log.info("Запуск Minecraft");
        log.info("Команда: " + cmd);

        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            log.error("Не удалось запустить процесс игры");
            running_ = false;
            return;
        }

        char buf[512];
        while (fgets(buf, sizeof(buf), pipe)) {
            string line(buf);
            while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
                line.pop_back();
            if (!line.empty()) {
                log.info("[MC] " + line);
                logs_.push_back(std::move(line));
            }
        }

        int rc = pclose(pipe);
        log.info("Minecraft завершился с кодом " + to_string(rc));
        running_ = false;
    }

    thread MinecraftLauncher::launch(const data::VersionDetails& vanilla,
                                     const data::VersionDetails* modloader,
                                     const unordered_map<string, string>* env) {
        const data::VersionDetails& active = modloader ? *modloader : vanilla;

        vector<string> args = buildCommand(active, vanilla);
        filesystem::path cwd = pathToMinecraft_;
        unordered_map<string, string> envMap = env ? *env : unordered_map<string, string>{};

        running_ = true;

        return thread([this, args = std::move(args), cwd = std::move(cwd), envMap = std::move(envMap)]() mutable {
            runProcess(args, cwd, envMap);
        });
    }
}
