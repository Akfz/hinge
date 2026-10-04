#include "core/download/MinecraftService.h"
#include "core/download/VanillaManifestService.h"
#include "core/install/InstallLock.h"
#include "core/log/LogBuffer.h"
#include "core/util/Maven.h"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <fstream>
#include <httplib.h>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <openssl/evp.h>
#include <thread>
#include <vector>

using namespace std;

namespace hinge::core::download {
    namespace {
        string currentOsName() {
            #ifdef _WIN32
                return "windows";
            #elif __APPLE__
                return "osx";
            #else
                return "linux";
            #endif
        }

        bool splitUrl(const string& url, string& base, string& path) {
            auto scheme = url.find("://");
            if (scheme == string::npos) return false;
            auto slash = url.find('/', scheme + 3);
            if (slash == string::npos) { base = url; path = "/"; }
            else { base = url.substr(0, slash); path = url.substr(slash); }
            return true;
        }

        string sha1File(const filesystem::path& p) {
            ifstream in(p, ios::binary);
            if (!in) return {};
            EVP_MD_CTX* ctx = EVP_MD_CTX_new();
            if (!ctx) return {};
            if (EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr) != 1) {
                EVP_MD_CTX_free(ctx);
                return {};
            }
            char buf[8192];
            while (in.read(buf, sizeof(buf)) || in.gcount() > 0)
                EVP_DigestUpdate(ctx, buf, static_cast<size_t>(in.gcount()));
            unsigned char digest[EVP_MAX_MD_SIZE];
            unsigned int digestLen = 0;
            EVP_DigestFinal_ex(ctx, digest, &digestLen);
            EVP_MD_CTX_free(ctx);
            char hex[65];
            for (unsigned int i = 0; i < digestLen; ++i)
                snprintf(hex + i * 2, 3, "%02x", digest[i]);
            return string(hex, digestLen * 2);
        }

        class RateLimiter {
            public:
                explicit RateLimiter(int perSecond)
                    : tokens_(perSecond), maxTokens_(perSecond),
                      lastRefill_(chrono::steady_clock::now()) {}
                void wait() {
                    unique_lock lk(mtx_);
                    while (true) {
                        auto now = chrono::steady_clock::now();
                        auto elapsed = chrono::duration_cast<chrono::milliseconds>(
                            now - lastRefill_).count();
                        int refill = static_cast<int>(elapsed) * maxTokens_ / 1000;
                        if (refill > 0) {
                            tokens_ = min(maxTokens_, tokens_ + refill);
                            lastRefill_ = now;
                        }
                        if (tokens_ > 0) { --tokens_; return; }
                        cv_.wait_for(lk, chrono::milliseconds(5));
                    }
                }
            private:
                mutex mtx_;
                condition_variable cv_;
                int tokens_;
                int maxTokens_;
                chrono::steady_clock::time_point lastRefill_;
        };

        RateLimiter g_limiter(500);

        void unzipNatives(const filesystem::path& jarPath, const filesystem::path& outDir) {
            filesystem::create_directories(outDir);
            #ifdef _WIN32
                string cmd = "tar -xf \"" + jarPath.string() + "\" -C \"" + outDir.string() + "\"";
            #else
                string cmd = "unzip -o -q \"" + jarPath.string() + "\" -d \"" + outDir.string() + "\" 2>/dev/null || "
                             "tar -xf \"" + jarPath.string() + "\" -C \"" + outDir.string() + "\"";
            #endif
            system(cmd.c_str());
        }

        using ByteCallback = function<void(int64_t)>;

        bool streamToFile(httplib::Client& cli, const string& path,
                          const filesystem::path& dest,
                          ByteCallback onBytes) {
            error_code ec;
            filesystem::create_directories(dest.parent_path(), ec);

            ofstream out;
            bool started = false;
            int status = 0;

            g_limiter.wait();

            auto res = cli.Get(path,
                [&](const httplib::Response& r) {
                    status = r.status;
                    if (r.status == 200) {
                        out.open(dest, ios::binary);
                        started = out.is_open();
                    }
                    return true;
                },
                [&](const char* data, size_t len) {
                    if (!started) return false;
                    if (install::InstallLock::instance().cancelRequested()) return false;
                    out.write(data, static_cast<streamsize>(len));
                    if (onBytes) onBytes(static_cast<int64_t>(len));
                    return true;
                });

            if (out.is_open()) out.close();

            if (!res) return false;
            if (status != 200) return false;
            if (!filesystem::exists(dest, ec)) return false;
            auto sz = filesystem::file_size(dest, ec);
            if (ec || sz == 0) { filesystem::remove(dest, ec); return false; }
            return true;
        }

        bool verifyFile(const filesystem::path& dest,
                        int64_t expectedSize, const string& expectedSha1) {
            error_code ec;
            if (expectedSize > 0 &&
                filesystem::file_size(dest, ec) != static_cast<uintmax_t>(expectedSize)) return false;
            if (!expectedSha1.empty() && sha1File(dest) != expectedSha1) return false;
            return true;
        }

        bool downloadFileWith(httplib::Client& cli, const string& path,
                              const filesystem::path& dest, int64_t expectedSize,
                              const string& expectedSha1, ByteCallback onBytes,
                              int maxAttempts = 5) {
            error_code ec;
            if (filesystem::exists(dest, ec) && verifyFile(dest, expectedSize, expectedSha1)) {
                if (onBytes && expectedSize > 0) onBytes(expectedSize);
                return true;
            }
            filesystem::remove(dest, ec);
            for (int attempt = 1; attempt <= maxAttempts; ++attempt) {
                if (install::InstallLock::instance().cancelRequested()) return false;
                if (streamToFile(cli, path, dest, onBytes) &&
                    verifyFile(dest, expectedSize, expectedSha1)) return true;
                error_code e2;
                if (filesystem::exists(dest, e2)) {
                    int64_t got = static_cast<int64_t>(filesystem::file_size(dest, e2));
                    if (got > 0 && onBytes) onBytes(-got);
                    filesystem::remove(dest, e2);
                }
                if (attempt < maxAttempts)
                    this_thread::sleep_for(chrono::milliseconds(250 * attempt));
            }
            return false;
        }

        bool tryDownloadFromUrl(const string& url, const filesystem::path& dest,
                                int64_t expectedSize, const string& expectedSha1,
                                ByteCallback onBytes) {
            string base, path;
            if (!splitUrl(url, base, path)) return false;
            httplib::Client cli(base);
            cli.enable_server_certificate_verification(false);
            cli.set_connection_timeout(15);
            cli.set_read_timeout(120);
            cli.set_follow_location(true);
            if (!streamToFile(cli, path, dest, onBytes)) return false;
            if (!verifyFile(dest, expectedSize, expectedSha1)) {
                error_code ec;
                filesystem::remove(dest, ec);
                return false;
            }
            return true;
        }

        bool downloadLibraryFile(const string& primaryUrl, const string& relPath,
                                 const filesystem::path& dest, int64_t expectedSize,
                                 const string& expectedSha1, ByteCallback onBytes) {
            error_code ec;
            if (filesystem::exists(dest, ec) && verifyFile(dest, expectedSize, expectedSha1)) {
                if (onBytes && expectedSize > 0) onBytes(expectedSize);
                return true;
            }
            vector<string> urlsToTry;
            if (!primaryUrl.empty()) urlsToTry.push_back(primaryUrl);
            static const vector<string> fallbackRepos = {
                "https://bmclapi2.bangbang93.com/maven/",
                "https://maven.quiltmc.org/repository/release/",
                "https://maven.quiltmc.org/repository/snapshot/",
                "https://repo1.maven.org/maven2/",
                "https://maven.fabricmc.net/",
                "https://libraries.minecraft.net/"
            };
            for (const auto& repo : fallbackRepos) {
                string cand = repo + relPath;
                if (find(urlsToTry.begin(), urlsToTry.end(), cand) == urlsToTry.end())
                    urlsToTry.push_back(cand);
            }
            for (const auto& u : urlsToTry) {
                if (install::InstallLock::instance().cancelRequested()) return false;
                if (tryDownloadFromUrl(u, dest, expectedSize, expectedSha1, onBytes)) return true;
            }
            log::LogBuffer::instance().error("Не удалось скачать библиотеку: " + relPath);
            return false;
        }
    }

    data::DownloaderInfo MinecraftService::snapshot() const {
        lock_guard lk(mtx_);
        auto copy = info_;
        copy.downloadedBytes = bytesDownloaded_.load();
        copy.totalBytes = bytesTotal_;
        return copy;
    }

    void MinecraftService::reset() {
        lock_guard lk(mtx_);
        info_ = data::DownloaderInfo{};
        bytesDownloaded_ = 0;
        bytesTotal_ = 0;
        phaseBytes_ = 0;
        phaseTotal_ = 0;
    }

    void MinecraftService::setProgress(int readyPercent, int downloadedCount, int totalFiles,
                                       const string& phase, const string& file) {
        lock_guard lk(mtx_);
        info_.readyPercent = readyPercent;
        info_.downloadedCount = downloadedCount;
        info_.totalFiles = totalFiles;
        info_.phase = phase;
        info_.file = file;
        info_.busy = true;
        info_.cancelling = install::InstallLock::instance().cancelRequested();
    }

    void MinecraftService::setPhaseDetail(const string& phase, const string& detail) {
        lock_guard lk(mtx_);
        if (!phase.empty()) info_.phase = phase;
        info_.detail = detail;
    }

    void MinecraftService::setCurrentFile(const string& name, const string& phase) {
        lock_guard lk(mtx_);
        info_.file = name;
        if (!phase.empty()) info_.phase = phase;
    }

    void MinecraftService::beginPhase(const string& phase, int64_t totalBytes) {
        lock_guard lk(mtx_);
        info_.phase = phase;
        info_.detail.clear();
        info_.currentFilePercent = 0;
        info_.currentBytes = 0;
        info_.currentTotalBytes = totalBytes;
        phaseBytes_ = 0;
        phaseTotal_ = totalBytes;
    }

    void MinecraftService::addBytes(int64_t n) {
        bytesDownloaded_.fetch_add(n);
        lock_guard lk(mtx_);
        phaseBytes_ += n;
        info_.currentBytes = phaseBytes_;
        if (phaseTotal_ > 0) {
            int pct = static_cast<int>(phaseBytes_ * 100 / phaseTotal_);
            if (pct > 100) pct = 100;
            info_.currentFilePercent = pct;
        }
    }

    void MinecraftService::tickDownloaded(int totalFiles) {
        lock_guard lk(mtx_);
        ++info_.downloadedCount;
        info_.totalFiles = totalFiles;
        info_.readyPercent = totalFiles > 0 ? (info_.downloadedCount * 100) / totalFiles : 100;
    }

    void MinecraftService::markDone() {
        lock_guard lk(mtx_);
        info_.readyPercent = 100;
        info_.currentFilePercent = 100;
        info_.phase = "Готово";
        info_.detail.clear();
        info_.file = "Done";
        info_.busy = false;
        info_.cancelling = false;
    }

    void MinecraftService::markError(const string& msg) {
        lock_guard lk(mtx_);
        info_.phase = "Ошибка";
        info_.detail = msg;
        info_.busy = false;
    }

    vector<string> MinecraftService::verifyVersion(const data::VersionDetails& details,
                                                   const filesystem::path& gameDir) const {
        vector<string> broken;
        error_code ec;

        bool isModLoader = details.id.rfind("fabric-loader-", 0) == 0 ||
                           details.id.rfind("quilt-loader-", 0) == 0 ||
                           details.id.rfind("neoforge-", 0) == 0 ||
                           details.id.find("-forge-") != string::npos ||
                           details.id.find("forge-") != string::npos ||
                           details.downloads.client.url.empty();

        if (!isModLoader) {
            filesystem::path clientJar = gameDir / "versions" / details.id / (details.id + ".jar");
            if (!filesystem::exists(clientJar, ec)) {
                broken.push_back(details.id + ".jar (missing)");
            } else {
                auto sz = filesystem::file_size(clientJar, ec);
                if (sz < 1024 * 1024) broken.push_back(details.id + ".jar (too small)");
                else if (details.downloads.client.size > 0 &&
                         sz != static_cast<uintmax_t>(details.downloads.client.size))
                    broken.push_back(details.id + ".jar (size mismatch)");
            }
        }

        for (const auto& lib : details.libraries) {
            string rel;
            int64_t size = 0;
            if (!lib.downloads.artifact.path.empty()) {
                rel  = lib.downloads.artifact.path;
                size = lib.downloads.artifact.size;
            } else if (!lib.name.empty()) {
                rel = util::mavenToPath(lib.name);
            }
            if (rel.empty()) continue;
            filesystem::path p = gameDir / "libraries" / rel;
            if (!filesystem::exists(p, ec)) {
                broken.push_back(p.filename().string() + " (missing)");
            } else if (size > 0 && filesystem::file_size(p, ec) != static_cast<uintmax_t>(size)) {
                broken.push_back(p.filename().string() + " (size mismatch)");
            }
        }
        return broken;
    }

    void MinecraftService::downloadVersion(const string& versionId,
                                           const filesystem::path& gameDir) {
        auto details = VanillaManifestService::fetchAndSave(versionId, gameDir);
        downloadVersion(details, gameDir);
    }

    void MinecraftService::downloadVersion(const data::VersionDetails& details,
                                           const filesystem::path& gameDir) {
        bool expected = false;
        if (!isDownloading_.compare_exchange_strong(expected, true)) {
            log::LogBuffer::instance().warn("Скачивание уже идёт, пропуск");
            return;
        }

        auto& log = log::LogBuffer::instance();
        auto& lock = install::InstallLock::instance();

        {
            lock_guard lk(mtx_);
            info_ = data::DownloaderInfo{};
            info_.phase = "Подготовка";
            info_.detail = "Версия " + details.id;
            info_.file = details.id;
            info_.busy = true;
        }
        bytesDownloaded_ = 0;
        bytesTotal_ = 0;
        phaseBytes_ = 0;
        phaseTotal_ = 0;

        log.info("=== Установка Minecraft " + details.id + " ===");

        try {
            data::AssetIndex assetIndex;
            bool hasAssetIndex = false;

            if (!details.assetIndex.id.empty()) {
                filesystem::path indexPath = gameDir / "assets" / "indexes" /
                                             (details.assetIndex.id + ".json");
                beginPhase("Индекс ассетов", 0);
                setCurrentFile("Index: " + details.assetIndex.id + ".json");
                log.info("Скачивание индекса ассетов: " + details.assetIndex.id);
                tryDownloadFromUrl(details.assetIndex.url, indexPath, 0,
                                   details.assetIndex.sha1, nullptr);

                if (filesystem::exists(indexPath)) {
                    try {
                        ifstream in(indexPath);
                        nlohmann::json j;
                        in >> j;
                        assetIndex = j.get<data::AssetIndex>();
                        hasAssetIndex = true;
                    } catch (const exception& e) {
                        log.warn(string("Не удалось разобрать индекс ассетов: ") + e.what());
                    }
                }
            }

            bool hasClient = !details.downloads.client.url.empty();

            struct LibTask {
                string url;
                string relPath;
                filesystem::path dest;
                int64_t size;
                string sha1;
            };
            vector<LibTask> libTasks;

            for (const auto& lib : details.libraries) {
                string relativePath;
                string url;
                int64_t size = 0;
                string sha1;
                if (!lib.downloads.artifact.path.empty()) {
                    relativePath = lib.downloads.artifact.path;
                    url = lib.downloads.artifact.url;
                    size = lib.downloads.artifact.size;
                    sha1 = lib.downloads.artifact.sha1;
                } else if (!lib.name.empty()) {
                    relativePath = util::mavenToPath(lib.name);
                    if (!lib.url.empty()) {
                        string baseRepo = lib.url;
                        if (baseRepo.back() != '/') baseRepo += '/';
                        url = baseRepo + relativePath;
                    }
                }
                if (relativePath.empty()) continue;
                libTasks.push_back({url, relativePath,
                                    gameDir / "libraries" / relativePath, size, sha1});
            }

            string osName = currentOsName();
            struct NativeTask {
                string url;
                filesystem::path destJar;
                int64_t size;
                string sha1;
            };
            vector<NativeTask> nativeTasks;

            for (const auto& lib : details.libraries) {
                auto it = lib.natives.find(osName);
                if (it == lib.natives.end()) continue;
                string classifier = it->second;
                auto clsIt = lib.downloads.classifiers.find(classifier);
                if (clsIt == lib.downloads.classifiers.end()) continue;
                const auto& native = clsIt->second;
                nativeTasks.push_back({native.url,
                                       gameDir / "libraries" / native.path,
                                       native.size, native.sha1});
            }

            int totalLibraries = static_cast<int>(details.libraries.size());
            int totalNatives = static_cast<int>(nativeTasks.size());
            int totalAssets = hasAssetIndex ? static_cast<int>(assetIndex.objects.size()) : 0;
            int totalFiles = (hasClient ? 1 : 0) + totalLibraries + totalNatives + totalAssets;

            {
                lock_guard lk(mtx_);
                info_.totalFiles = totalFiles;
            }

            int64_t totalBytes = 0;
            if (hasClient) totalBytes += details.downloads.client.size;
            for (const auto& t : libTasks) if (t.size > 0) totalBytes += t.size;
            for (const auto& t : nativeTasks) if (t.size > 0) totalBytes += t.size;
            if (hasAssetIndex)
                for (const auto& [k, v] : assetIndex.objects) totalBytes += v.size;
            bytesTotal_ = totalBytes;

            log.info("Всего файлов: " + to_string(totalFiles) +
                     ", общий размер: " + to_string(totalBytes / (1024 * 1024)) + " MB");

            if (hasClient) {
                if (lock.cancelRequested()) throw runtime_error("Отменено");

                filesystem::path clientJar = gameDir / "versions" / details.id /
                                             (details.id + ".jar");
                beginPhase("Загрузка клиента", details.downloads.client.size);
                setCurrentFile("client.jar");
                log.info("Скачивание клиента (" +
                         to_string(details.downloads.client.size / (1024 * 1024)) + " MB)");

                tryDownloadFromUrl(details.downloads.client.url, clientJar,
                                   details.downloads.client.size,
                                   details.downloads.client.sha1,
                                   [&](int64_t n) { addBytes(n); });
                tickDownloaded(totalFiles);
            }

            if (!libTasks.empty()) {
                if (lock.cancelRequested()) throw runtime_error("Отменено");

                int64_t libTotal = 0;
                for (const auto& t : libTasks) if (t.size > 0) libTotal += t.size;

                beginPhase("Библиотеки", libTotal);
                setPhaseDetail("Библиотеки", "0 / " + to_string(libTasks.size()));
                log.info("Скачивание библиотек (" + to_string(libTasks.size()) + ")");

                atomic<size_t> libNext{0};
                atomic<int> libDone{0};
                int libThreads = min(downloadThreads_, (int)libTasks.size());
                vector<thread> libWorkers;

                for (int i = 0; i < libThreads; ++i) {
                    libWorkers.emplace_back([&] {
                        while (true) {
                            if (lock.cancelRequested()) return;
                            size_t idx = libNext.fetch_add(1);
                            if (idx >= libTasks.size()) return;
                            const auto& task = libTasks[idx];
                            setCurrentFile(task.dest.filename().string(), "Библиотеки");
                            downloadLibraryFile(task.url, task.relPath, task.dest,
                                                task.size, task.sha1,
                                                [&](int64_t n) { addBytes(n); });
                            tickDownloaded(totalFiles);
                            int done = ++libDone;
                            setPhaseDetail("Библиотеки",
                                to_string(done) + " / " + to_string(libTasks.size()));
                        }
                    });
                }
                for (auto& t : libWorkers) t.join();
            }

            if (!nativeTasks.empty()) {
                if (lock.cancelRequested()) throw runtime_error("Отменено");

                int64_t natTotal = 0;
                for (const auto& t : nativeTasks) if (t.size > 0) natTotal += t.size;

                beginPhase("Нативные модули", natTotal);
                setPhaseDetail("Нативные модули", "0 / " + to_string(nativeTasks.size()));
                log.info("Скачивание нативных модулей (" +
                         to_string(nativeTasks.size()) + ")");

                atomic<size_t> natNext{0};
                int natThreads = min(downloadThreads_, (int)nativeTasks.size());
                vector<thread> natWorkers;

                for (int i = 0; i < natThreads; ++i) {
                    natWorkers.emplace_back([&] {
                        while (true) {
                            if (lock.cancelRequested()) return;
                            size_t idx = natNext.fetch_add(1);
                            if (idx >= nativeTasks.size()) return;
                            const auto& task = nativeTasks[idx];
                            setCurrentFile(task.destJar.filename().string(), "Нативные модули");
                            tryDownloadFromUrl(task.url, task.destJar, task.size,
                                               task.sha1,
                                               [&](int64_t n) { addBytes(n); });
                            tickDownloaded(totalFiles);
                        }
                    });
                }
                for (auto& t : natWorkers) t.join();

                if (lock.cancelRequested()) throw runtime_error("Отменено");

                beginPhase("Распаковка нативных модулей", 0);
                log.info("Распаковка нативных модулей");
                filesystem::path nativesDir = gameDir / "bin" / "natives";
                filesystem::create_directories(nativesDir);
                for (const auto& task : nativeTasks)
                    unzipNatives(task.destJar, nativesDir);
            }

            if (hasAssetIndex && !assetIndex.objects.empty()) {
                if (lock.cancelRequested()) throw runtime_error("Отменено");

                int64_t assetsTotal = 0;
                for (const auto& [k, v] : assetIndex.objects) assetsTotal += v.size;

                beginPhase("Ассеты", assetsTotal);
                setPhaseDetail("Ассеты", "0 / " + to_string(assetIndex.objects.size()));
                log.info("Скачивание ассетов (" +
                         to_string(assetIndex.objects.size()) + ")");

                vector<data::AssetObject> assets;
                assets.reserve(assetIndex.objects.size());
                for (auto& [k, v] : assetIndex.objects) assets.push_back(v);

                atomic<size_t> next{0};
                atomic<int> assetsDone{0};
                vector<thread> workers;

                for (int i = 0; i < downloadThreads_; ++i) {
                    workers.emplace_back([&] {
                        httplib::Client cli("https://resources.download.minecraft.net");
                        cli.enable_server_certificate_verification(false);
                        cli.set_keep_alive(true);
                        cli.set_connection_timeout(20);
                        cli.set_read_timeout(120);

                        while (true) {
                            if (lock.cancelRequested()) return;
                            size_t idx = next.fetch_add(1);
                            if (idx >= assets.size()) return;
                            const auto& asset = assets[idx];
                            if (asset.hash.size() < 2) { tickDownloaded(totalFiles); continue; }
                            string prefix = asset.hash.substr(0, 2);
                            string path = "/" + prefix + "/" + asset.hash;
                            filesystem::path dest = gameDir / "assets" / "objects" /
                                                    prefix / asset.hash;
                            downloadFileWith(cli, path, dest, asset.size, asset.hash,
                                             [&](int64_t n) { addBytes(n); });
                            tickDownloaded(totalFiles);
                            int done = ++assetsDone;
                            if (done % 50 == 0)
                                setPhaseDetail("Ассеты",
                                    to_string(done) + " / " + to_string(assets.size()));
                        }
                    });
                }
                for (auto& t : workers) t.join();
            }

            markDone();
            log.info("=== Установка Minecraft " + details.id + " завершена ===");
        } catch (const exception& e) {
            markError(e.what());
            log.error(string("Ошибка установки: ") + e.what());
            isDownloading_ = false;
            throw;
        } catch (...) {
            markError("Неизвестная ошибка");
            isDownloading_ = false;
            throw;
        }

        isDownloading_ = false;
    }
}
