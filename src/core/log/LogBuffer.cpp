#include "core/log/LogBuffer.h"

#include <chrono>

using namespace std;

namespace hinge::core::log {
    LogBuffer& LogBuffer::instance() {
        static LogBuffer inst;
        return inst;
    }

    void LogBuffer::push(Level level, const string& msg) {
        lock_guard lk(mtx_);
        auto now = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()).count();
        entries_.push_back({nextSeq_++, level, msg, now});
        while (entries_.size() > kMaxEntries) entries_.pop_front();
    }

    void LogBuffer::info(const string& m)  { push(Level::Info, m); }
    void LogBuffer::warn(const string& m)  { push(Level::Warn, m); }
    void LogBuffer::error(const string& m) { push(Level::Error, m); }

    vector<Entry> LogBuffer::since(uint64_t lastSeq) const {
        lock_guard lk(mtx_);
        vector<Entry> out;
        for (const auto& e : entries_) if (e.seq > lastSeq) out.push_back(e);
        return out;
    }

    void LogBuffer::clear() {
        lock_guard lk(mtx_);
        entries_.clear();
    }

    uint64_t LogBuffer::lastSeq() const {
        lock_guard lk(mtx_);
        return nextSeq_ - 1;
    }
}
