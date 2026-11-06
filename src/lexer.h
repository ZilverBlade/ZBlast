#pragma once

#include "token.h"

namespace zblast {
    Token NextToken(const char *&YYCURSOR);

    struct AspectLexerState {
        bool expectType = false;   // after synthetic/inherited/collection
        bool expectAspectID = false;  // after "aspect"
        bool expectBrace = false;  // after "aspect <id>": next '{' is a plain token
        bool expectExpr = false;   // after "=>"
    };

    Token NextToken_Aspect(const char *&YYCURSOR, AspectLexerState& state);

    static void AdvanceLocation(int &line, int &col, const char *start, const char *end) {
        for (const char *p = start; p < end; ++p) {
            if (*p == '\n') {
                line++;
                col = 1;
            } else {
                col++;
            }
        }
    }

} // zblast
