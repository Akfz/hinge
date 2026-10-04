#pragma once

#include <filesystem>
#include <string>

using namespace std;

namespace hinge::core::java {
    class JavaService {
        public:
            static JavaService& instance() {
                static JavaService inst;
                return inst;
            }

            filesystem::path ensureJava(int majorVersion);
            bool removeJava(int majorVersion);
            filesystem::path findInstalled(int majorVersion);

            static int resolveJavaVersion(int declaredMajor, const string& mcVersion);

        private:
            JavaService() = default;

            filesystem::path findExecutable(const filesystem::path& dir);
            static string currentOs();
            static string currentArch();
    };
}
