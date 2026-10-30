#pragma once
#include "lexer.h"
#include "parsetree.h"
#include <memory>

namespace zblast {
    class Parser {
    public:
        std::unique_ptr<parsetree::Start> Parse(Token *pTokens);

        [[nodiscard]] const parsetree::Start *GetTree() const { return m_Tree.get(); }

        [[maybe_unused]] Token Accept(ETokenType expected);

        [[nodiscard]] ETokenType Peek();

        [[nodiscard]] int CurrLine();

        [[nodiscard]] int CurrCol();

    private:
        Token *m_Ptr = nullptr;
        bool m_ReachedEof = false;
        std::unique_ptr<parsetree::Start> m_Tree;
    };
} // zblast

