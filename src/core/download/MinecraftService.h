#pragma once

#include "core/data/DownloaderInfo.h"
#include "core/data/Minecraft.h"

#include <atomic>
#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

using namespace std;

namespace hinge::core::download {
    class MinecraftService {
        public:
            static MinecraftService& instance() {
                static MinecraftService inst;
                return inst;
            }

            MinecraftService(const MinecraftService&)            = delete;
            MinecraftService& operator=(const MinecraftService&) = delete;

            void setDownloadThreads(int n) { downloadThreads_ = n; }
            int getDownloadThreads() const { return downloadThreads_; }

            void setProgress(int readyPercent, int downloadedCount, int totalFiles,
                             const string& phase, const string& file);
            void setPhaseDetail(const string& phase, const string& detail);
            void setCurrentFile(const string& name, const string& phase = "");
            void beginPhase(const string& phase, int64_t totalBytes);
            void addBytes(int64_t n);

            void downloadVersion(const string& versionId,
                                 const filesystem::path& gameDir);
            void downloadVersion(const data::VersionDetails& details,
                                 const filesystem::path& gameDir);

            vector<string> verifyVersion(const data::VersionDetails& details,
                                         const filesystem::path& gameDir) const;

            data::DownloaderInfo snapshot() const;
            void reset();

        private:
            MinecraftService() = default;

            void tickDownloaded(int totalFiles);
            void markDone();
            void markError(const string& msg);

            mutable mutex mtx_;
            data::DownloaderInfo info_;

            atomic<bool> isDownloading_{false};
            atomic<int64_t> bytesDownloaded_{0};
            int64_t bytesTotal_ = 0;
            int64_t phaseBytes_ = 0;
            int64_t phaseTotal_ = 0;
            int downloadThreads_ = 8;
    };
}
