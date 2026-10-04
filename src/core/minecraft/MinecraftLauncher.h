#pragma once

#include "core/data/Minecraft.h"
#include "core/data/Profile.h"

#include <filesystem>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

using namespace std;

namespace hinge::core::minecraft {
    class MinecraftLauncher {
        public:
            MinecraftLauncher(filesystem::path minecraftDir, data::Profile& profile);

            thread launch(const data::VersionDetails& vanilla,
                          const data::VersionDetails* modloader = nullptr,
                          const unordered_map<string, string>* env = nullptr);

            const vector<string>& logs() const {
                return logs_;
            }
            bool isRunning() const {
                return running_;
            }

        private:
            filesystem::path pathToJava_;
            filesystem::path pathToMinecraft_;
            data::Profile* loadFrom_;

            vector<string> logs_;
            bool running_ = false;

            void collectLibraries(const data::VersionDetails& details,
                                  vector<filesystem::path>& out);
            string getLibraryPath(const data::Library& lib);
            bool isLibraryAllowed(const data::Library& lib);
            bool isArgumentAllowed(const nlohmann::json& argObj);

            vector<string> buildCommand(const data::VersionDetails& active,
                                        const data::VersionDetails& vanilla);
            vector<string> buildJvmArgs(const data::VersionDetails& active,
                                        const data::VersionDetails& vanilla,
                                        const string& classpath);
            vector<string> buildGameArgs(const data::VersionDetails& active,
                                         const data::VersionDetails& vanilla,
                                         const string& assetIndexId);

            void parseJvmArgs(const vector<nlohmann::json>& raw,
                              vector<string>& out,
                              const data::VersionDetails& vanilla,
                              const string& classpath);
            void parseGameArgs(const vector<nlohmann::json>& raw,
                               vector<string>& out,
                               const data::VersionDetails& active,
                               const string& assetIndexId);

            string substituteJvm(const string& arg,
                                 const data::VersionDetails& vanilla,
                                 const string& classpath);
            string substituteGame(const string& arg,
                                  const data::VersionDetails& active,
                                  const string& assetIndexId);
            void ensureEssentialJvmArgs(vector<string>& jvmArgs);

            void runProcess(const vector<string>& args,
                            const filesystem::path& cwd,
                            const unordered_map<string, string>& env);

            static string findJavaExecutable();
            static string shellQuote(const string& s);
            static string currentOs();
            static bool isQuickPlayArg(const string& arg);
    };
}
