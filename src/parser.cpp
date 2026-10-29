#include "parser.h"

#include <cstdio>
#include <stdexcept>

namespace zblast {
    void Parser::Parse(Token *pTokens) {
        m_Ptr = pTokens;
    }

    void Parser::Accept(ETokenType expected) {
        ETokenType actual = Peek();
        if (actual != expected) {
            char errBuf[128];
            snprintf(errBuf, sizeof(errBuf), "Expected token %s but got %s!", ETokenType_ToString(expected),
                     ETokenType_ToString(actual));
            throw std::runtime_error(errBuf);
        }
    }

    ETokenType Parser::Peek() {
        return m_Ptr->type;
    }
}
