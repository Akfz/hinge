#include "core/settings/SettingsStore.h"

#include <fstream>
#include <iostream>

using namespace std;

namespace hinge::core::settings {
    SettingsStore::SettingsStore(filesystem::path file)
        : file_(std::move(file)) {
        load();
    }

    void SettingsStore::load() {
        if (!filesystem::exists(file_)) {
            cout << "Settings: creating default at " << file_ << '\n';
            save();
            return;
        }

        try {
            ifstream in(file_);
            nlohmann::json j;
            in >> j;
            settings_ = j.get<data::LauncherSettings>();
        } catch (const exception& e) {
            cerr << "Settings: cannot read (" << e.what()
                 << "), falling back to defaults\n";
            settings_ = data::LauncherSettings{};
        }
    }

    void SettingsStore::save() const {
        error_code ec;
        filesystem::create_directories(file_.parent_path(), ec);

        try {
            nlohmann::json j = settings_;
            ofstream out(file_);
            out << j.dump(4);
        } catch (const exception& e) {
            cerr << "Settings: cannot save: " << e.what() << '\n';
        }
    }

    void SettingsStore::setPort(int port) {
        if (port != -1 && (port < 1 || port > 65535)) port = -1;
        settings_.port = port;
        save();
    }

    void SettingsStore::setCloseWithSite(bool v) {
        settings_.closeWithSite = v;
        save();
    }

    void SettingsStore::setAutoOpenBrowser(bool v) {
        settings_.autoOpenBrowser = v;
        save();
    }

    void SettingsStore::update(const function<void(data::LauncherSettings&)>& mutator) {
        mutator(settings_);
        save();
    }
}
