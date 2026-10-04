#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

namespace hinge::core::data {
    struct Profile {
        filesystem::path profilePath;

        string nameOfProfile;
        string minecraftName = "name";
        string uuid;
        string accessToken = "0";
        string refreshToken;
        string ico;

        bool unlicensed = true;
        int listQueue = 0;

        vector<string> minecraftPaths;
        string selectedMinecraftPath;

        unordered_map<string, string> javaPaths;
        unordered_map<string, int> maxMemory;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Profile,
            nameOfProfile, minecraftName, uuid,
            accessToken, refreshToken, ico,
            unlicensed, listQueue,
            minecraftPaths, selectedMinecraftPath,
            javaPaths, maxMemory)

        optional<filesystem::path> getActiveJavaPath() const {
            auto key = getActiveMinecraftPath().string();
            auto it = javaPaths.find(key);
            if (it == javaPaths.end() || it->second.empty()) return nullopt;
            return filesystem::path(it->second);
        }

        filesystem::path getActiveMinecraftPath() const {
            if (!selectedMinecraftPath.empty()) return selectedMinecraftPath;
            if (!minecraftPaths.empty()) return minecraftPaths.front();
            return profilePath / "minecraft";
        }

        int getActiveMaxMemory() const {
            auto key = getActiveMinecraftPath().string();
            auto it = maxMemory.find(key);
            return it != maxMemory.end() ? it->second : 2;
        }
    };

    inline ostream& operator<<(ostream& os, const Profile& p) {
        os << "Profile{name='" << p.nameOfProfile
           << "', mcName='" << p.minecraftName
           << "', uuid=" << p.uuid
           << ", path=" << p.getActiveMinecraftPath().string()
           << ", memory=" << p.getActiveMaxMemory()
           << ", type=" << (p.unlicensed ? "unlicensed" : "licensed")
           << '}';
        return os;
    }
}
