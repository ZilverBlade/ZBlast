#pragma once

#include "token.h"

namespace zblast {
    Token NextToken(const char *&YYCURSOR);

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
