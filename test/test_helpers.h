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
inline bool TokensEqual(const Token& lhs, const Token& rhs) {
    return lhs.type == rhs.type && lhs.text == rhs.text;
}

/// Tokenize an entire null-terminated string into a vector of Tokens.
/// The vector will always end with the EToken_EOF token.
inline std::vector<Token> Tokenize(const char* src) {
    std::vector<Token> tokens;
    if (!src) {
        tokens.push_back(Token{ EToken_EOF, {} });
        return tokens;
    }

    const char* cursor = src;
    while (true) {
        Token tok = NextToken(cursor);
        tokens.push_back(tok);
        if (tok.type == EToken_EOF) {
            break;
        }
    }
    return tokens;
}

inline std::vector<Token> Tokenize(const std::string& src) {
    return Tokenize(src.c_str());
}

/// Helper to extract just the token types from a list of tokens
inline std::vector<ETokenType> ExtractTypes(const std::vector<Token>& tokens) {
    std::vector<ETokenType> types;
    types.reserve(tokens.size());
    for (const auto& tok : tokens) {
        types.push_back(tok.type);
    }
    return types;
}

/// Resolves path to a file in the test_files directory
inline std::filesystem::path GetTestFilePath(const std::string& filename) {
#ifdef TEST_FILES_DIR
    std::filesystem::path base(TEST_FILES_DIR);
    if (std::filesystem::exists(base / filename)) {
        return base / filename;
    }
#endif
    // Fallbacks if TEST_FILES_DIR is not defined or relative path differs
    std::vector<std::filesystem::path> candidates = {
        std::filesystem::path("test_files") / filename,
        std::filesystem::path("../test_files") / filename,
        std::filesystem::path("../../test_files") / filename
    };

    for (const auto& p : candidates) {
        if (std::filesystem::exists(p)) {
            return p;
        }
    }

    throw std::runtime_error("Could not find test file: " + filename);
}

/// Read test file contents into a string
inline std::string ReadTestFile(const std::string& filename) {
    std::filesystem::path path = GetTestFilePath(filename);
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open test file: " + path.string());
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

} // namespace zblast::test

// GoogleTest custom printers so failure messages show human-readable tokens
namespace zblast {
    inline void PrintTo(const Token& tok, std::ostream* os) {
        *os << "Token(" << ETokenType_ToString(tok.type);
        if (!tok.text.empty()) {
            *os << ", \"" << tok.text << "\"";
        }
        *os << ")";
    }

    inline bool operator==(const Token& lhs, const Token& rhs) {
        return lhs.type == rhs.type && lhs.text == rhs.text;
    }
}
