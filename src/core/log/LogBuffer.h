#pragma once

#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <vector>

using namespace std;

namespace hinge::core::log {
    enum class Level { Info, Warn, Error };

    struct Entry {
        uint64_t seq;
        Level level;
        string message;
        int64_t timestamp;
    };

    class LogBuffer {
        public:
            static LogBuffer& instance();

            void push(Level level, const string& msg);
            void info(const string& m);
            void warn(const string& m);
            void error(const string& m);

            vector<Entry> since(uint64_t lastSeq) const;
            void clear();
            uint64_t lastSeq() const;

        private:
            LogBuffer() = default;
            mutable mutex mtx_;
            deque<Entry> entries_;
            uint64_t nextSeq_ = 1;
            static constexpr size_t kMaxEntries = 2000;
    };
}
