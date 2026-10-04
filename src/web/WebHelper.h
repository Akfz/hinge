#pragma once

#include "nlohmann/json_fwd.hpp"
#include "web/CommandRegistry.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <httplib.h>
#include <string>
#include <thread>

using namespace std;

namespace hinge::web {
    class WebHelper {
        public:
            using JsonHandler = function<nlohmann::json(const nlohmann::json&)>;

            CommandRegistry& commands() { return commands_; }

            const string& host() const { return host_; }
            int port() const { return port_; }
            const string& url() const { return url_; }

            explicit WebHelper(string webDir);
            ~WebHelper();

            void get(const string& path, JsonHandler handler);
            void post(const string& path, JsonHandler handler);

            string start(const string& host, int port);
            void stop();

            static void openInBrowser(const string& url);

        private:
            CommandRegistry commands_;
            void watchLoop();
            uint64_t computeStamp() const;

            string webDir_;
            string host_;
            int port_ = 0;
            string url_;
            httplib::Server server_;
            atomic<uint64_t> buildVersion_{1};
            atomic<bool> stopWatcher_{false};
            thread watcher_;
            thread serverThread_;
    };
}
