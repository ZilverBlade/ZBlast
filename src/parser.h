#pragma once
#include "lexer.h"

namespace zblast {
    class Parser {
    public:
        void Parse(Token* pTokens);
        void Accept(ETokenType expected);
        ETokenType Peek();

    private:

        Token* m_Ptr = nullptr;
    };
} // zblast