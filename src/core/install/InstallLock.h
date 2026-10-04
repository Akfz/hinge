#pragma once

#include <atomic>
#include <mutex>
#include <string>

using namespace std;

namespace hinge::core::install {
    class InstallLock {
        public:
            static InstallLock& instance();

            bool tryAcquire(const string& label);
            void release();

            bool isBusy() const;
            string label() const;

            void requestCancel();
            bool cancelRequested() const;

        private:
            InstallLock() = default;
            atomic<bool> busy_{false};
            atomic<bool> cancel_{false};
            mutable mutex mtx_;
            string label_;
    };
}
