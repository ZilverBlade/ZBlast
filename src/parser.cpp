#include "parser.h"

#include <cassert>
#include <cstdio>
#include <vector>
#include <stdexcept>
#include <string>
#include <memory>

using Str = std::string;

template<typename T>
using Ptr = std::unique_ptr<T>;

template<typename T>
using List = std::vector<T>;

using std::make_unique;

namespace zblast {
#define PARSER_PANIC() do {\
    char errBuf[128];\
    snprintf(errBuf, sizeof(errBuf), "Failed to match parse token %s in rule %s! @ %d:%d", ETokenType_ToString(parser.Peek()), __func__, parser.CurrLine(), parser.CurrCol());\
    throw std::runtime_error(errBuf);\
} while (false)

    static Ptr<parsetree::Iden> Parse_Iden(Parser &parser) {
        Ptr node = make_unique<parsetree::Iden>();
        parser.FillAstData(*node);
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_L_ANGLE) {
            node->IsLiteral = true;
            parser.Accept(EToken_L_ANGLE);
            node->ID = parser.Accept(EToken_ID).text;
            parser.Accept(EToken_R_ANGLE);
            return node;
        }
        if (token == EToken_ID) {
            node->IsLiteral = false;
            node->ID = parser.Accept(EToken_ID).text;
            return node;
        }

        PARSER_PANIC();
    }

    static Ptr<parsetree::LabeledIden> Parse_LabeledIden(Parser &parser) {
        Ptr node = make_unique<parsetree::LabeledIden>();
        parser.FillAstData(*node);
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_LABEL) {
            node->HasLabel = true;
            node->LABEL = parser.Accept(EToken_LABEL).text;
            parser.Accept(EToken_COLON);
            node->iden = Parse_Iden(parser);
            return node;
        } else {
            node->HasLabel = false;
            node->iden = Parse_Iden(parser);
            return node;
        }
    }

    static Ptr<parsetree::VariantQual> Parse_VariantQual(Parser &parser) {
        Ptr node = make_unique<parsetree::VariantQual>();
        parser.FillAstData(*node);
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_PIPE) {
            node->IsEpsilon = false;
            parser.Accept(EToken_PIPE);
            node->labeled_iden = Parse_LabeledIden(parser);
            node->variant_qual = Parse_VariantQual(parser);
            return node;
        } else {
            node->IsEpsilon = true;
            return node;   
        }
    }

    static Ptr<parsetree::ChildQual> Parse_ChildQual(Parser &parser) {
        Ptr node = make_unique<parsetree::ChildQual>();
        parser.FillAstData(*node);
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_STAR) {
            node->Mode = parsetree::ChildQual::Mode_List;
            parser.Accept(EToken_STAR);
            return node;
        }
        if (token == EToken_PIPE) {
            node->Mode = parsetree::ChildQual::Mode_Variant;
            node->variant_qual = Parse_VariantQual(parser);
            return node;
        }
        // FOLLOW
        if (token == EToken_L_BRACKET || token == EToken_L_ANGLE || token == EToken_STAR
            || token == EToken_ID || token == EToken_LABEL || token == EToken_SEMICOLON) {
            node->Mode = parsetree::ChildQual::IsEpsilon;
            return node;
        }

        PARSER_PANIC();
    }

    static Ptr<parsetree::Child> Parse_Child(Parser &parser) {
        Ptr node = make_unique<parsetree::Child>();
        parser.FillAstData(*node);
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_L_ANGLE || token == EToken_ID || token == EToken_LABEL) {
            node->Mode = parsetree::Child::Mode_IdentifierWQualifier;
            node->labeled_iden = Parse_LabeledIden(parser);
            node->child_qual = Parse_ChildQual(parser);
            return node;
        }
        if (token == EToken_L_BRACKET) {
            node->Mode = parsetree::Child::Mode_Optional;
            parser.Accept(EToken_L_BRACKET);
            node->labeled_iden = Parse_LabeledIden(parser);
            parser.Accept(EToken_R_BRACKET);
            return node;
        }

        PARSER_PANIC();
    }

    static Ptr<parsetree::ChildList> Parse_ChildList(Parser &parser) {
        Ptr node = make_unique<parsetree::ChildList>();
        parser.FillAstData(*node);
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_LABEL || token == EToken_ID || token == EToken_L_BRACKET
            || token == EToken_L_ANGLE) {
            node->IsEpsilon = false;
            node->child = Parse_Child(parser);
            node->child_list = Parse_ChildList(parser);
            return node;
        }
        // FOLLOW
        if (token == EToken_SEMICOLON) {
            node->IsEpsilon = true;
            return node;
        }

        PARSER_PANIC();
    }

    static Ptr<parsetree::NodeRef> Parse_NodeRef(Parser &parser) {
        Ptr node = make_unique<parsetree::NodeRef>();
        parser.FillAstData(*node);
        node->ID = parser.Accept(EToken_ID).text;
        return node;
    }

    static Ptr<parsetree::OptInherit> Parse_OptInherit(Parser &parser) {
        Ptr node = make_unique<parsetree::OptInherit>();
        parser.FillAstData(*node);
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_COLON) {
            node->IsEpsilon = false;
            parser.Accept(EToken_COLON);
            node->node_ref = Parse_NodeRef(parser);
            return node;
        }

        // FOLLOW
        if (token == EToken_DOUBLE_COLON_EQ || token == EToken_SEMICOLON) {
            node->IsEpsilon = true;
            return node;
        }

        PARSER_PANIC();
    }

    static Ptr<parsetree::Node> Parse_Node(Parser &parser) {
        Ptr node = make_unique<parsetree::Node>();
        parser.FillAstData(*node);
        node->node_ref = Parse_NodeRef(parser);
        node->opt_inherit = Parse_OptInherit(parser);
        return node;
    }

    static Ptr<parsetree::OptExpansion> Parse_OptExpansion(Parser &parser) {
        Ptr node = make_unique<parsetree::OptExpansion>();
        parser.FillAstData(*node);
        ETokenType token = parser.Peek();

        // FIRST
        if (token == EToken_DOUBLE_COLON_EQ) {
            node->IsEpsilon = false;
            parser.Accept(EToken_DOUBLE_COLON_EQ);
            node->child_list = Parse_ChildList(parser);
            return node;
        }
        // FOLLOW
        if (token == EToken_SEMICOLON) {
            // Other case is node
            node->IsEpsilon = true;
            return node;
        }
        PARSER_PANIC();
    }

    static Ptr<parsetree::Rule> Parse_Rule(Parser &parser) {
        Ptr node = make_unique<parsetree::Rule>();
        parser.FillAstData(*node);
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_ABSTRACT) {
            node->IsAbstract = true;
            parser.Accept(EToken_ABSTRACT);
            node->node = Parse_Node(parser);
            parser.Accept(EToken_SEMICOLON);
        } else {
            // Other case is node
            node->IsAbstract = false;
            node->node = Parse_Node(parser);
            node->opt_expansion = Parse_OptExpansion(parser);
            parser.Accept(EToken_SEMICOLON);
        }
        return node;
    }

    static Ptr<parsetree::RuleList> Parse_RuleList(Parser &parser) {
        Ptr node = make_unique<parsetree::RuleList>();
        parser.FillAstData(*node);
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_ID || token == EToken_ABSTRACT) {
            node->IsEpsilon = false;
            node->rule = Parse_Rule(parser);
            node->rule_list = Parse_RuleList(parser);
            return node;
        }
        // FOLLOW
        if (token == EToken_EOF) {
            node->IsEpsilon = true;
            return node;
        }

        PARSER_PANIC();
    }

    static Ptr<parsetree::Start> Parse_Start(Parser &parser) {
        Ptr node = make_unique<parsetree::Start>();
        parser.FillAstData(*node);
        node->rule_list = Parse_RuleList(parser);
        parser.Accept(EToken_EOF);
        return node;
    }

    std::unique_ptr<parsetree::Start> Parser::Parse(Token *pTokens) {
        m_Ptr = pTokens;
        m_ReachedEof = false;
        m_Tree = Parse_Start(*this);
        return std::move(m_Tree);
    }

    Token Parser::Accept(ETokenType expected) {
        ETokenType actual = Peek();
        if (actual != expected) {
            char errBuf[128];
            snprintf(errBuf, sizeof(errBuf), "Expected token %s but got %s! @ %d:%d", ETokenType_ToString(expected),
                     ETokenType_ToString(actual), CurrLine(), CurrCol());
            throw std::runtime_error(errBuf);
        }
        Token token = *m_Ptr;
        if (actual == EToken_EOF) {
            m_ReachedEof = true;
            m_Ptr = nullptr;
        } else {
            m_Ptr++;
        }
        return token;
    }

    ETokenType Parser::Peek() {
        if (m_ReachedEof) {
            throw std::runtime_error("Trying to read beyond EOF!");
        }
        assert(m_Ptr);
        return m_Ptr->type;
    }


    int Parser::CurrLine() {
        if (m_ReachedEof) {
            return -1;
        }
        assert(m_Ptr);
        return m_Ptr->lineStart;
    }

    int Parser::CurrCol() {
        if (m_ReachedEof) {
            return -1;
        }
        assert(m_Ptr);
        return m_Ptr->colStart;
    }

    void Parser::FillAstData(parsetree::ParseNode& node) {
        node.LineStart = m_Ptr->lineStart;
        node.ColumnStart = m_Ptr->colStart;
        node.LineEnd = m_Ptr->lineEnd;
        node.ColumnEnd = m_Ptr->colEnd;
    }
}
