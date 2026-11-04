#pragma once
#include "lexer.h"
#include "parsetree.h"
#include <memory>

namespace zblast {
    class ParserException : public std::exception {
    public:
        ParserException(int line, int col, ETokenType expectedToken, Token receivedToken);
        ParserException(int line, int col, ETokenType failToken, const char* rule);
        ~ParserException();

        const char* what() const noexcept override;

        int Line() const;
        int Column() const;
        ETokenType Expected() const;
        const Token& Received() const;
        bool HasExpectedToken() const;
        const char* Rule() const;
    private:
        std::string m_Message;
        int m_Line;
        int m_Col;
        ETokenType m_ExpectedToken;
        Token m_ReceivedToken;
        bool m_HasExpectedToken;
        const char* m_Rule = nullptr;
    };

    class Parser {
    public:
        std::unique_ptr<parsetree::Start> Parse(Token *pTokens);

        [[nodiscard]] const parsetree::Start *GetTree() const { return m_Tree.get(); }

        [[maybe_unused]] Token Accept(ETokenType expected);

        [[nodiscard]] ETokenType Peek();

        [[nodiscard]] int CurrLine();

        [[nodiscard]] int CurrCol();
        
        void FillAstData(parsetree::AstNode& node);

    private:
        Token *m_Ptr = nullptr;
        bool m_ReachedEof = false;
        std::unique_ptr<parsetree::Start> m_Tree;
    };
} // zblast

