// Gemini Flash 3.8 High AI Generated @ 2026-10-29

#pragma once

#include "lexer.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <iostream>

namespace zblast::test {
    /// Comparison operator for Token if not defined in token.h
    inline bool TokensEqual(const Token &lhs, const Token &rhs) {
        return lhs.type == rhs.type && lhs.text == rhs.text;
    }

    /// Tokenize an entire null-terminated string into a vector of Tokens.
    /// The vector will always end with the EToken_EOF token.
    inline std::vector<Token> Tokenize(const char *src) {
        std::vector<Token> tokens;
        if (!src) {
            tokens.push_back(Token{EToken_EOF, {}});
            return tokens;
        }

        const char *cursor = src;
        int l = 1, c = 1;
        while (true) {
            const char *start = cursor;
            Token tok = NextToken(cursor);
            AdvanceLocation(l, c, start, cursor);
            tok.colEnd = c;
            tok.lineEnd = l;
            tok.lineStart = l;
            tok.colStart = tok.colEnd - static_cast<int>(tok.text.length());
            tokens.push_back(tok);
            if (tok.type == EToken_EOF) {
                break;
            }
        }
        return tokens;
    }

    inline std::vector<Token> Tokenize(const std::string &src) {
        return Tokenize(src.c_str());
    }

    /// Helper to extract just the token types from a list of tokens
    inline std::vector<ETokenType> ExtractTypes(const std::vector<Token> &tokens) {
        std::vector<ETokenType> types;
        types.reserve(tokens.size());
        for (const auto &tok: tokens) {
            types.push_back(tok.type);
        }
        return types;
    }

    /// Resolves base directory for test files
    inline std::filesystem::path GetTestDir() {
#ifdef TEST_FILES_DIR
        return std::filesystem::path(TEST_FILES_DIR);
#else
        for (const auto &p : { std::filesystem::path("test_files"),
                               std::filesystem::path("../test_files"),
                               std::filesystem::path("../../test_files") }) {
            if (std::filesystem::exists(p)) return p;
        }
        return std::filesystem::path("test_files");
#endif
    }

    /// Resolves path to a file in the test_files directory
    inline std::filesystem::path GetTestFilePath(const std::string &filename) {
        std::filesystem::path base = GetTestDir();
        if (std::filesystem::exists(base / filename)) {
            return base / filename;
        }
        if (std::filesystem::exists(base / "lexer" / filename)) {
            return base / "lexer" / filename;
        }
        if (std::filesystem::exists(base / "parser" / filename)) {
            return base / "parser" / filename;
        }
        if (std::filesystem::exists(base / "ast" / filename)) {
            return base / "ast" / filename;
        }

        // Fallbacks if relative path differs
        std::vector<std::filesystem::path> candidates = {
            std::filesystem::path("test_files") / filename,
            std::filesystem::path("test_files/lexer") / filename,
            std::filesystem::path("test_files/parser") / filename,
            std::filesystem::path("test_files/ast") / filename,
            std::filesystem::path("../test_files") / filename,
            std::filesystem::path("../test_files/lexer") / filename,
            std::filesystem::path("../test_files/parser") / filename,
            std::filesystem::path("../test_files/ast") / filename,
            std::filesystem::path("../../test_files") / filename,
            std::filesystem::path("../../test_files/lexer") / filename,
            std::filesystem::path("../../test_files/parser") / filename,
            std::filesystem::path("../../test_files/ast") / filename
        };

        for (const auto &p: candidates) {
            if (std::filesystem::exists(p)) {
                return p;
            }
        }

        throw std::runtime_error("Could not find test file: " + filename);
    }

    /// Read test file contents into a string
    inline std::string ReadTestFile(const std::string &filename) {
        std::filesystem::path path = GetTestFilePath(filename);
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open test file: " + path.string());
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }
} // namespace test

// GoogleTest custom printers so failure messages show human-readable tokens
namespace zblast {
    inline void PrintTo(const Token &tok, std::ostream *os) {
        *os << "Token(" << ETokenType_ToString(tok.type);
        if (!tok.text.empty()) {
            *os << ", \"" << tok.text << "\"";
        }
        *os << " @ " << tok.lineStart << ":" << tok.colStart;
        *os << ")";
    }

    inline void PrintTo(const ETokenType &toktype, std::ostream *os) {
        *os << ETokenType_ToString(toktype);
    }

    inline bool operator==(const Token &lhs, const Token &rhs) {
        return lhs.type == rhs.type && lhs.text == rhs.text;
    }
}
