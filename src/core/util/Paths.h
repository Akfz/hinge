#pragma once

#include <filesystem>

#if defined(_WIN32)
    extern "C" __declspec(dllimport)
    unsigned long __stdcall GetModuleFileNameA(
        void* hModule,
        char* lpFilename,
        unsigned long nSize);

    #ifndef MAX_PATH
        #define MAX_PATH 260
    #endif
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
#else
    #include <unistd.h>
    #include <limits.h>
#endif

namespace hinge::core::util {

inline std::filesystem::path getExecutablePath() {
#ifdef _WIN32
    char buf[MAX_PATH];
    unsigned long n = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    if (n > 0 && n < MAX_PATH)
        return std::filesystem::path(std::string(buf, n));
    return {};

#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::vector<char> buf(size + 1, '\0');
    if (_NSGetExecutablePath(buf.data(), &size) == 0) {
        std::error_code ec;
        auto p = std::filesystem::canonical(buf.data(), ec);
        return ec ? std::filesystem::path(buf.data()) : p;
    }
    return {};

#else
    char buf[PATH_MAX];
    ssize_t n = ::readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        return std::filesystem::path(buf);
    }
    return {};
#endif
}

inline std::filesystem::path getExecutableDir() {
    auto exe = getExecutablePath();
    if (!exe.empty()) return exe.parent_path();
    return std::filesystem::current_path();
}

}
