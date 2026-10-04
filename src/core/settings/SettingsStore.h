#pragma once

#include "core/data/Settings.h"
#include <filesystem>
#include <functional>

using namespace std;

namespace hinge::core::settings {
    class SettingsStore {
        public:
            explicit SettingsStore(filesystem::path file);

            const data::LauncherSettings& get() const {
                return settings_;
            }

            void setPort(int port);
            void setCloseWithSite(bool v);
            void setAutoOpenBrowser(bool v);

            void update(const function<void(data::LauncherSettings&)>& mutator);

            void load();
            void save() const;

            const filesystem::path& path() const {
                return file_;
            }
        private:
            filesystem::path file_;
            data::LauncherSettings settings_;
    };
}
