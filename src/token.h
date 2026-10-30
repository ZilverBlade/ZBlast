#pragma once

#include <string_view>

namespace zblast {

#define TOKENS(X) \
    X(ABSTRACT) \
    X(ID) X(LABEL) \
    X(COLON) \
    X(DOUBLE_COLON_EQ) \
    X(SEMICOLON) \
    X(STAR) \
    X(PIPE) \
    X(L_BRACKET) X(R_BRACKET) \
    X(L_ANGLE) X(R_ANGLE) \
    X(EOF) \
    X(ERR)

#define EXPAND_TOKEN_ENUM(x) EToken_##x,
    enum ETokenType {
        TOKENS(EXPAND_TOKEN_ENUM)
    };
#undef EXPAND_TOKEN_ENUM
#define EXPAND_TOKEN_TOSTRING(x) case EToken_##x: return #x;
    static const char* ETokenType_ToString(ETokenType token) {
        switch (token) {
            TOKENS(EXPAND_TOKEN_TOSTRING)
        default:
            return "???";
        }
    }
#undef EXPAND_TOKEN_TOSTRING
#undef TOKENS

    struct Token {
        ETokenType type;
        std::string_view text;
        int line;
        int col;
    };
} // zblast
