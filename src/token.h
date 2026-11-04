#pragma once

#include <string>
#include <string_view>

namespace zblast {

#define TOKENS(X) \
    X(ABSTRACT, "abstract") \
    X(ID, "<identifier>") X(LABEL, "<label>") \
    X(COLON, ":") \
    X(DOUBLE_COLON_EQ, "::=") \
    X(SEMICOLON, ";") \
    X(STAR, "*") \
    X(PIPE, "|") \
    X(L_BRACKET, "[") X(R_BRACKET, "]") \
    X(L_ANGLE, "<") X(R_ANGLE, ">") \
    X(EOF, "<end of file>") \
    X(ERR, "<error>")

#define EXPAND_TOKEN_ENUM(x, ...) EToken_##x,
    enum ETokenType {
        TOKENS(EXPAND_TOKEN_ENUM)
    };
#undef EXPAND_TOKEN_ENUM
#define EXPAND_TOKEN_TOSTRING(x, ...) case EToken_##x: return #x;
#define EXPAND_TOKEN_TOSTRING2(x, y, ...) case EToken_##x: return y;
    static const char* ETokenType_ToString(ETokenType token) {
        switch (token) {
            TOKENS(EXPAND_TOKEN_TOSTRING)
        default:
            return "???";
        }
    }
    static const char* ETokenType_ToString_Literal(ETokenType token) {
        switch (token) {
            TOKENS(EXPAND_TOKEN_TOSTRING2)
        default:
            return "???";
        }
    }
#undef EXPAND_TOKEN_TOSTRING
#undef EXPAND_TOKEN_TOSTRING2
#undef TOKENS

    struct Token {
        ETokenType type;
        std::string_view text;
        int lineStart;
        int colStart;
        int lineEnd;
        int colEnd;
    };

    static std::string Token_ToString(const Token& token) {
        if (token.type == EToken_ERR || token.type == EToken_ID || token.type == EToken_LABEL) {
            return std::string(token.text);
        } else {
            return ETokenType_ToString_Literal(token.type);
        }
    }
} // zblast
