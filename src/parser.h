#pragma once
#include "lexer.h"

namespace zblast {
    class Parser {
    public:
        void Parse(Token *pTokens);

        [[maybe_unused]] Token Accept(ETokenType expected);

        [[nodiscard]] ETokenType Peek();

        [[nodiscard]] int CurrLine();

        [[nodiscard]] int CurrCol();

    private:
        Token *m_Ptr = nullptr;
        bool m_ReachedEof = false;
    };
} // zblast
