#include "core/install/InstallLock.h"

using namespace std;

namespace hinge::core::install {
    InstallLock& InstallLock::instance() {
        static InstallLock inst;
        return inst;
    }

    bool InstallLock::tryAcquire(const string& lbl) {
        bool expected = false;
        if (!busy_.compare_exchange_strong(expected, true)) return false;
        lock_guard lk(mtx_);
        label_ = lbl;
        cancel_ = false;
        return true;
    }

    void InstallLock::release() {
        lock_guard lk(mtx_);
        busy_ = false;
        cancel_ = false;
        label_.clear();
    }

    bool InstallLock::isBusy() const {
        return busy_;
    }

    string InstallLock::label() const {
        lock_guard lk(mtx_);
        return label_;
    }

    void InstallLock::requestCancel() {
        cancel_ = true;
    }

    bool InstallLock::cancelRequested() const {
        return cancel_;
    }
}
