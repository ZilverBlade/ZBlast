#pragma once
#include <string>
#include <streambuf>
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

        class FileBuf : public std::streambuf {
        public:
            explicit FileBuf(FILE* f) : m_File(f) {}

        protected:
            int_type overflow(int_type c) override {
                if (c != traits_type::eof() && std::fputc(c, m_File) == EOF)
                    return traits_type::eof();
                return traits_type::not_eof(c);
            }
            std::streamsize xsputn(const char* s, std::streamsize n) override {
                return std::fwrite(s, 1, n, m_File);
            }
            
        private:
            FILE* m_File;
        };
    } // namespace util
} // namespace zblast 