#include "core/profile/ProfileManager.h"

#include <openssl/evp.h>

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>

using namespace std;

namespace hinge::core::profile {
    ProfileManager::ProfileManager(filesystem::path profilesRoot, settings::SettingsStore& settings)
        : profilesRoot_(std::move(profilesRoot))
        , settings_(settings) {}

    string ProfileManager::safeFolderName(const string& name) {
        string out = name;
        for (char& c : out) {
            if (c == '\\' || c == '/' || c == ':' || c == '*' ||
                c == '?'  || c == '"' || c == '<' || c == '>' || c == '|') {
                c = '_';
            }
        }
        return out;
    }

    string ProfileManager::offlineUuid(const string& name) {
        string input = "OfflinePlayer:" + name;
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int len = 0;

        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (ctx) {
            EVP_DigestInit_ex(ctx, EVP_md5(), nullptr);
            EVP_DigestUpdate(ctx, input.data(), input.size());
            EVP_DigestFinal_ex(ctx, hash, &len);
            EVP_MD_CTX_free(ctx);
        }

        hash[6] = static_cast<unsigned char>((hash[6] & 0x0F) | 0x30);
        hash[8] = static_cast<unsigned char>((hash[8] & 0x3F) | 0x80);

        char buf[40];
        snprintf(buf, sizeof(buf),
            "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            hash[0],  hash[1],  hash[2],  hash[3],
            hash[4],  hash[5],  hash[6],  hash[7],
            hash[8],  hash[9],  hash[10], hash[11],
            hash[12], hash[13], hash[14], hash[15]);
        return buf;
    }

    filesystem::path ProfileManager::folderFor(const string& name) const {
        return profilesRoot_ / safeFolderName(name);
    }

    void ProfileManager::save(data::Profile& p) {
        try {
            filesystem::create_directories(p.profilePath);
            filesystem::path configPath = p.profilePath / "profile.json";
            ofstream(configPath) << nlohmann::json(p).dump(4);
        } catch (const exception& e) {
            cerr << "Failed to save profile '" << p.nameOfProfile
                 << "': " << e.what() << '\n';
        }
    }

    void ProfileManager::load() {
        profiles_.clear();
        activeName_.clear();

        error_code ec;
        filesystem::create_directories(profilesRoot_, ec);
        if (ec) {
            cerr << "Cannot create profiles folder: " << ec.message() << '\n';
            return;
        }

        for (auto& entry : filesystem::directory_iterator(profilesRoot_, ec)) {
            if (!entry.is_directory()) continue;

            filesystem::path configPath = entry.path() / "profile.json";
            if (!filesystem::exists(configPath)) continue;

            try {
                ifstream in(configPath);
                nlohmann::json j;
                in >> j;

                data::Profile p = j.get<data::Profile>();
                p.profilePath = entry.path();
                profiles_.push_back(std::move(p));
            } catch (const exception& e) {
                cerr << "Failed to load profile from "
                     << configPath.string() << ": " << e.what() << '\n';
            }
        }

        sortByQueue();

        const auto& cfg = settings_.get();
        if (!cfg.lastActiveProfileName.empty() && exists(cfg.lastActiveProfileName)) {
            activeName_ = cfg.lastActiveProfileName;
        } else if (!profiles_.empty()) {
            activeName_ = profiles_.front().nameOfProfile;
            persistActiveName();
        }
    }

    void ProfileManager::sortByQueue() {
        sort(profiles_.begin(), profiles_.end(),
            [](const data::Profile& a, const data::Profile& b) {
                return a.listQueue < b.listQueue;
            });
    }

    void ProfileManager::persistActiveName() {
        settings_.update([&](data::LauncherSettings& s) {
            s.lastActiveProfileName = activeName_;
        });
    }

    data::Profile* ProfileManager::find(const string& name) {
        for (auto& p : profiles_) {
            if (p.nameOfProfile == name) return &p;
        }
        return nullptr;
    }

    bool ProfileManager::exists(const string& name) const {
        for (const auto& p : profiles_) {
            if (p.nameOfProfile == name) return true;
        }
        return false;
    }

    data::Profile* ProfileManager::active() {
        if (activeName_.empty()) return nullptr;
        return find(activeName_);
    }

    const data::Profile* ProfileManager::active() const {
        if (activeName_.empty()) return nullptr;
        for (const auto& p : profiles_) {
            if (p.nameOfProfile == activeName_) return &p;
        }
        return nullptr;
    }

    void ProfileManager::setActive(const string& name) {
        if (!exists(name)) throw runtime_error("Profile not found: " + name);
        activeName_ = name;
        persistActiveName();
    }

    data::Profile& ProfileManager::createOffline(const string& name) {
        if (exists(name)) {
            throw ProfileExistException("Profile '" + name + "' already exists");
        }

        filesystem::path folder = folderFor(name);
        if (filesystem::exists(folder)) {
            throw ProfileExistException("Folder for profile '" + name + "' already exists on disk");
        }

        data::Profile p;
        p.nameOfProfile = name;
        p.minecraftName = name;
        p.uuid = offlineUuid(name);
        p.profilePath = folder;
        p.unlicensed = true;

        string mcPath = (folder / "minecraft").string();
        p.minecraftPaths.push_back(mcPath);
        p.selectedMinecraftPath = mcPath;
        p.maxMemory[mcPath] = 2;

        save(p);
        profiles_.push_back(std::move(p));

        if (activeName_.empty()) {
            activeName_ = name;
            persistActiveName();
        }

        return profiles_.back();
    }

    data::Profile& ProfileManager::createMicrosoft(const string& profileName,
                                                   const string& minecraftName,
                                                   const string& uuid,
                                                   const string& accessToken,
                                                   const string& refreshToken) {
        if (exists(profileName)) {
            throw ProfileExistException("Profile '" + profileName + "' already exists");
        }

        filesystem::path folder = folderFor(profileName);
        if (filesystem::exists(folder)) {
            throw ProfileExistException("Folder for profile '" + profileName + "' already exists on disk");
        }

        data::Profile p;
        p.nameOfProfile = profileName;
        p.minecraftName = minecraftName;
        p.uuid = uuid;
        p.accessToken = accessToken;
        p.refreshToken = refreshToken;
        p.profilePath = folder;
        p.unlicensed = false;

        string mcPath = (folder / "minecraft").string();
        p.minecraftPaths.push_back(mcPath);
        p.selectedMinecraftPath = mcPath;
        p.maxMemory[mcPath] = 2;

        save(p);
        profiles_.push_back(std::move(p));

        activeName_ = profileName;
        persistActiveName();

        return profiles_.back();
    }

    void ProfileManager::remove(const string& name) {
        auto it = find_if(profiles_.begin(), profiles_.end(),
            [&](const data::Profile& p) { return p.nameOfProfile == name; });

        if (it == profiles_.end()) {
            throw runtime_error("Profile not found: " + name);
        }

        filesystem::path folder = it->profilePath;
        profiles_.erase(it);

        error_code ec;
        filesystem::remove_all(folder, ec);
        if (ec) {
            cerr << "Failed to delete profile folder: " << ec.message() << '\n';
        }

        if (activeName_ == name) {
            activeName_ = profiles_.empty() ? "" : profiles_.front().nameOfProfile;
            persistActiveName();
        }
    }

    void ProfileManager::markPlayed(const string& name) {
        for (auto& p : profiles_) {
            if (p.nameOfProfile == name) p.listQueue = 0;
            else p.listQueue += 1;
            save(p);
        }
        sortByQueue();
    }
}
