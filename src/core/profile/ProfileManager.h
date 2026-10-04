#pragma once

#include "core/data/Profile.h"
#include "core/settings/SettingsStore.h"

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace hinge::core::profile {
    class ProfileExistException : public runtime_error {
        public:
            using runtime_error::runtime_error;
    };

    class ProfileManager {
        public:
            ProfileManager(filesystem::path profilesRoot, settings::SettingsStore& settings);

            void load();

            data::Profile& createOffline(const string& name);
            data::Profile& createMicrosoft(const string& profileName,
                                           const string& minecraftName,
                                           const string& uuid,
                                           const string& accessToken,
                                           const string& refreshToken);

            void save(data::Profile& p);
            void remove(const string& name);
            void markPlayed(const string& name);

            data::Profile* active();
            const data::Profile* active() const;
            void setActive(const string& name);

            data::Profile* find(const string& name);
            bool exists(const string& name) const;

            vector<data::Profile>& all() {
                return profiles_;
            }
            const vector<data::Profile>& all() const {
                return profiles_;
            }
        private:
            filesystem::path profilesRoot_;
            settings::SettingsStore& settings_;
            vector<data::Profile> profiles_;
            string activeName_;

            void sortByQueue();
            void persistActiveName();
            filesystem::path folderFor(const string& name) const;

            static string safeFolderName(const string& name);
            static string offlineUuid(const string& name);
    };
}
