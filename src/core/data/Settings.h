#pragma once

#include <nlohmann/json.hpp>
#include <string>

using namespace std;

namespace hinge::core::data {
    struct LauncherSettings {
        int port = 12345;
        bool closeWithSite = true;
        bool autoOpenBrowser = true;
        int downloadThreads = 20;

        string lastActiveProfileName;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(LauncherSettings,
            port, closeWithSite, autoOpenBrowser,
            lastActiveProfileName, downloadThreads)
    };
}
