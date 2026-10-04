#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

namespace hinge::core::data {
    struct Version {
        string id;
        string type;
        string url;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Version, id, type, url)
    };

    struct Latest {
        string release;
        string snapshot;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Latest, release, snapshot)
    };

    struct VersionManifest {
        Latest latest;
        vector<Version> versions;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VersionManifest, latest, versions)
    };

    struct JavaVersionInfo {
        string component;
        int32_t majorVersion = 0;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(JavaVersionInfo, component, majorVersion)
    };

    struct ArgumentsInfo {
        vector<nlohmann::json> game;
        vector<nlohmann::json> jvm;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ArgumentsInfo, game, jvm)
    };

    struct AssetObject {
        string hash;
        int64_t size = 0;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AssetObject, hash, size)
    };

    struct AssetIndex {
        unordered_map<string, AssetObject> objects;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AssetIndex, objects)
    };

    struct AssetIndexInfo {
        string url;
        string id;
        string sha1;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AssetIndexInfo, url, id, sha1)
    };

    struct DownloadInfo {
        string url;
        string sha1;
        int64_t size = 0;
        string path;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(DownloadInfo, url, sha1, size, path)
    };

    struct Downloads {
        DownloadInfo client;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Downloads, client)
    };

    struct LibraryDownloads {
        DownloadInfo artifact;
        unordered_map<string, DownloadInfo> classifiers;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LibraryDownloads, artifact, classifiers)
    };

    struct Rule {
        string action;
        nlohmann::json os = nullptr;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Rule, action, os)
    };

    struct Library {
        LibraryDownloads downloads;
        string name;
        string url;
        unordered_map<string, string> natives;
        vector<Rule> rules;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Library, downloads, name, url, natives, rules)
    };

    struct VersionDetails {
        string id;
        string mainClass;
        Downloads downloads;
        vector<Library> libraries;
        AssetIndexInfo assetIndex;
        JavaVersionInfo javaVersion;
        ArgumentsInfo arguments;
        string minecraftArguments;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VersionDetails,
            id, mainClass, downloads, libraries,
            assetIndex, javaVersion, arguments, minecraftArguments)
    };
}
