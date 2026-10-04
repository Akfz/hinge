#pragma once

#include <filesystem>
#include <string>

using namespace std;

namespace hinge::core::modloader {
    class ModloaderInstaller {
        public:
            static void installFabric(const string& mcVersion,
                                      const string& loaderVersion,
                                      const filesystem::path& gameDir);

            static void installQuilt(const string& mcVersion,
                                     const string& loaderVersion,
                                     const filesystem::path& gameDir);

            static void installForge(const string& mcVersion,
                                     const string& forgeVersion,
                                     const filesystem::path& gameDir);

            static void installNeoForge(const string& loaderVersion,
                                        const filesystem::path& gameDir);

        private:
            ModloaderInstaller() = delete;

            static void installJsonModloader(const string& url,
                                             const string& profileId,
                                             const filesystem::path& gameDir,
                                             const string& loaderName);

            static void downloadAndRunInstaller(const string& url,
                                                const filesystem::path& gameDir,
                                                const string& installerName,
                                                const string& javaPath = "");

            static string findJavaExecutable();
            static int runProcess(const string& cmd);
    };
}
