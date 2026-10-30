#pragma once

#include "token.h"

namespace zblast {
    struct LexerState {
        int line;
        int col;
    };
    Token NextToken(const char *&YYCURSOR, LexerState* state);
} // zblast
