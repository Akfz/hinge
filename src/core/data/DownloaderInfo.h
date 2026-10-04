#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

using namespace std;

namespace hinge::core::data {
    struct DownloaderInfo {
        int readyPercent = 0;
        int currentFilePercent = 0;
        int downloadedCount = 0;
        int totalFiles = 0;

        int64_t totalBytes = 0;
        int64_t downloadedBytes = 0;
        int64_t currentBytes = 0;
        int64_t currentTotalBytes = 0;

        string phase;
        string detail;
        string file;

        bool busy = false;
        bool cancelling = false;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(DownloaderInfo,
            readyPercent, currentFilePercent, downloadedCount, totalFiles,
            totalBytes, downloadedBytes, currentBytes, currentTotalBytes,
            phase, detail, file, busy, cancelling)
    };
}
