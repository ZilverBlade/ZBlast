#pragma once
#include <string>
#include <cstdio>

namespace zblast {
    namespace util {
        template <typename ...Args>
        static std::string Format(const char* fmt, Args&&... args) {
            int capacity = snprintf(nullptr, 0, fmt, args...);
            std::string buf(capacity, '\000');
            snprintf(buf.data(), buf.max_size(), fmt, args...);
            return buf;
        }
    } // namespace util
} // namespace zblast 