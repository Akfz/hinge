#include "core/java/JavaService.h"
#include "core/util/Paths.h"

#include <cstdlib>
#include <iostream>
#include <system_error>

using namespace std;

namespace hinge::core::java {
    string JavaService::currentOs() {
        #ifdef _WIN32
            return "windows";
        #elif __APPLE__
            return "mac";
        #else
            return "linux";
        #endif
    }

    string JavaService::currentArch() {
        #if defined(__x86_64__) || defined(_M_X64)
            return "x64";
        #elif defined(__aarch64__) || defined(_M_ARM64)
            return "aarch64";
        #else
            return "x64";
        #endif
    }

    int JavaService::resolveJavaVersion(int declaredMajor, const string& mcVersion) {
        if (declaredMajor > 0) {
            if (declaredMajor <= 8) return 8;
            if (declaredMajor <= 17) return 17;
            return 21;
        }

        if (mcVersion.rfind("1.20.5", 0) == 0 || mcVersion.rfind("1.20.6", 0) == 0 ||
            mcVersion.rfind("1.21", 0) == 0) {
            return 21;
        }
        if (mcVersion.rfind("1.17", 0) == 0 || mcVersion.rfind("1.18", 0) == 0 ||
            mcVersion.rfind("1.19", 0) == 0 || mcVersion.rfind("1.20", 0) == 0) {
            return 17;
        }
        return 8;
    }

    filesystem::path JavaService::findExecutable(const filesystem::path& dir) {
        error_code ec;
        if (!filesystem::exists(dir, ec)) return {};

        #ifdef _WIN32
            const string target = "javaw.exe";
        #else
            const string target = "java";
        #endif

        for (const auto& entry : filesystem::recursive_directory_iterator(dir, ec)) {
            if (entry.is_regular_file(ec) && entry.path().filename() == target) {
                #ifndef _WIN32
                    filesystem::permissions(entry.path(),
                        filesystem::perms::owner_exec | filesystem::perms::group_exec | filesystem::perms::others_exec,
                        filesystem::perm_options::add, ec);
                #endif
                return entry.path();
            }
        }
        return {};
    }

    filesystem::path JavaService::ensureJava(int majorVersion) {
        filesystem::path runtimesDir = util::getExecutableDir() / "runtimes";
        filesystem::path targetDir   = runtimesDir / ("java-" + to_string(majorVersion));

        filesystem::path existingExe = findExecutable(targetDir);
        if (!existingExe.empty()) return existingExe;

        cout << "=== Portable Java " << majorVersion << " not found. Downloading Adoptium Temurin... ===\n";

        filesystem::create_directories(targetDir);

        string os   = currentOs();
        string arch = currentArch();
        string ext  = (os == "windows") ? "zip" : "tar.gz";

        string url = "https://api.adoptium.net/v3/binary/latest/" +
                     to_string(majorVersion) +
                     "/ga/" + os + "/" + arch + "/jdk/hotspot/normal/eclipse";

        filesystem::path archivePath = runtimesDir / ("java-" + to_string(majorVersion) + "-archive." + ext);

        string downloadCmd = "curl -L -f -# -o \"" + archivePath.string() + "\" \"" + url + "\"";
        cout << "Downloading: " << url << '\n';
        int dlCode = system(downloadCmd.c_str());

        if (dlCode != 0 || !filesystem::exists(archivePath) || filesystem::file_size(archivePath) < 1000000) {
            error_code ec;
            filesystem::remove(archivePath, ec);
            throw runtime_error("Failed to download Portable Java " + to_string(majorVersion));
        }

        cout << "Extracting Java " << majorVersion << "...\n";
        string extractCmd;
        if (os == "windows") {
            extractCmd = "tar -xf \"" + archivePath.string() + "\" -C \"" + targetDir.string() + "\"";
        } else {
            extractCmd = "tar -xzf \"" + archivePath.string() + "\" -C \"" + targetDir.string() + "\"";
        }

        int extCode = system(extractCmd.c_str());
        error_code ec;
        filesystem::remove(archivePath, ec);

        if (extCode != 0) {
            throw runtime_error("Failed to extract Java " + to_string(majorVersion));
        }

        filesystem::path javaExe = findExecutable(targetDir);
        if (javaExe.empty()) {
            throw runtime_error("Java binary not found in extracted folder: " + targetDir.string());
        }

        cout << "=== Portable Java " << majorVersion << " ready: " << javaExe << " ===\n";
        return javaExe;
    }

    filesystem::path JavaService::findInstalled(int majorVersion) {
        filesystem::path dir = util::getExecutableDir() / "runtimes"
                            / ("java-" + to_string(majorVersion));
        return findExecutable(dir);
    }

    bool JavaService::removeJava(int majorVersion) {
        filesystem::path dir = util::getExecutableDir() / "runtimes"
                            / ("java-" + to_string(majorVersion));
        error_code ec;
        if (!filesystem::exists(dir, ec)) return false;
        filesystem::remove_all(dir, ec);
        return !ec;
    }
}
