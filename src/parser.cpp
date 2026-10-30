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
    namespace parsetree {
        struct ParseNode {
            virtual const char *NodeName() const = 0;

            virtual List<ParseNode *> Children() const { return {}; }
            virtual List<Str> Values() const { return {}; }
        };

        struct NodeRef : ParseNode {
            Str ID;
            const char *NodeName() const override { return "NodeRef"; }
            List<Str> Values() const override { return {ID}; }
        };

        struct OptInherit : ParseNode {
            bool isEpsilon;
            Ptr<NodeRef> node_ref;
            const char *NodeName() const override { return "OptInherit"; }
            List<ParseNode *> Children() const { return {node_ref.get()}; }
        };

        struct Node : ParseNode {
            Ptr<NodeRef> node_ref;
            Ptr<OptInherit> opt_inherit;
            const char *NodeName() const override { return "Node"; }
            List<ParseNode *> Children() const { return {node_ref.get(), opt_inherit.get()}; }
        };

        struct Child;

        struct Iden : ParseNode {
            bool isLiteral;
            Str ID;
            const char *NodeName() const override { return "Iden"; }
            List<Str> Values() const override { return {ID}; }
        };

        struct LabeledIden : ParseNode {
            bool hasLabel;
            Str LABEL;
            Ptr<Iden> iden;
            const char *NodeName() const override { return "LabeledIden"; }
            List<ParseNode *> Children() const { return {iden.get()}; }
            List<Str> Values() const override { return {LABEL}; }
        };

        struct ChildQual : ParseNode {
            enum {
                Mode_List,
                Mode_Variant,
                IsEpsilon,
            } Mode;

            Ptr<Child> child;
            const char *NodeName() const override { return "ChildQual"; }

            List<ParseNode *> Children() const;
        };

        struct Child : ParseNode {
            enum {
                Mode_IdentifierWQualifier,
                Mode_Optional,
            } Mode;

            Ptr<LabeledIden> labeled_iden;
            Ptr<ChildQual> child_qual;

            const char *NodeName() const override { return "Child"; }
            List<ParseNode *> Children() const { return {labeled_iden.get(), child_qual.get()}; }
        };

        List<ParseNode *> ChildQual::Children() const { return {child.get()}; }

        struct ChildList : ParseNode {
            bool isEpsilon;
            Ptr<Child> child;
            Ptr<ChildList> child_list;

            const char *NodeName() const override { return "ChildList"; }
            List<ParseNode *> Children() const { return {child.get(), child_list.get()}; }
        };

        struct Rule : ParseNode {
            bool isAbstract;
            Ptr<Node> node;
            Ptr<ChildList> child_list;

            const char *NodeName() const override { return "Rule"; }
            List<ParseNode *> Children() const { return {node.get(), child_list.get()}; }
        };

        struct RuleList : ParseNode {
            bool isEpsilon;
            Ptr<Rule> rule;
            Ptr<RuleList> rule_list;

            const char *NodeName() const override { return "RuleList"; }
            List<ParseNode *> Children() const { return {rule.get(), rule_list.get()}; }
        };

        struct Start : ParseNode {
            Ptr<RuleList> rule_list;

            const char *NodeName() const override { return "Start"; }
            List<ParseNode *> Children() const { return {rule_list.get()}; }
        };
    }

#define PARSER_PANIC() do {\
    char errBuf[128];\
    snprintf(errBuf, sizeof(errBuf), "Failed to match parse token %s in rule %s! @ %d:%d", ETokenType_ToString(parser.Peek()), __func__, parser.CurrLine(), parser.CurrCol());\
    throw std::runtime_error(errBuf);\
} while (false)

    static Ptr<parsetree::Iden> Parse_Iden(Parser &parser) {
        Ptr node = make_unique<parsetree::Iden>();
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_L_ANGLE) {
            node->isLiteral = true;
            parser.Accept(EToken_L_ANGLE);
            node->ID = parser.Accept(EToken_ID).text;
            parser.Accept(EToken_R_ANGLE);
            return node;
        }
        if (token == EToken_ID) {
            node->isLiteral = false;
            node->ID = parser.Accept(EToken_ID).text;
            return node;
        }

        PARSER_PANIC();
    }

    static Ptr<parsetree::LabeledIden> Parse_LabeledIden(Parser &parser) {
        Ptr node = make_unique<parsetree::LabeledIden>();
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_LABEL) {
            node->hasLabel = true;
            node->LABEL = parser.Accept(EToken_LABEL).text;
            parser.Accept(EToken_COLON);
            node->iden = Parse_Iden(parser);
            return node;
        } else {
            node->iden = Parse_Iden(parser);
            return node;
        }
    }

    static Ptr<parsetree::Child> Parse_Child(Parser &parser);

    static Ptr<parsetree::ChildQual> Parse_ChildQual(Parser &parser) {
        Ptr node = make_unique<parsetree::ChildQual>();
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_STAR) {
            node->Mode = parsetree::ChildQual::Mode_List;
            parser.Accept(EToken_STAR);
            return node;
        }
        if (token == EToken_PIPE) {
            node->Mode = parsetree::ChildQual::Mode_Variant;
            parser.Accept(EToken_PIPE);
            node->child = Parse_Child(parser);
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
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_LABEL || token == EToken_ID || token == EToken_L_BRACKET
            || token == EToken_L_ANGLE) {
            node->isEpsilon = false;
            node->child = Parse_Child(parser);
            node->child_list = Parse_ChildList(parser);
            return node;
        }
        // FOLLOW
        if (token == EToken_SEMICOLON) {
            node->isEpsilon = true;
            return node;
        }

        PARSER_PANIC();
    }

    static Ptr<parsetree::NodeRef> Parse_NodeRef(Parser &parser) {
        Ptr node = make_unique<parsetree::NodeRef>();
        node->ID = parser.Accept(EToken_ID).text;
        return node;
    }

    static Ptr<parsetree::OptInherit> Parse_OptInherit(Parser &parser) {
        Ptr node = make_unique<parsetree::OptInherit>();
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_COLON) {
            node->isEpsilon = false;
            parser.Accept(EToken_COLON);
            node->node_ref = Parse_NodeRef(parser);
            return node;
        }

        // FOLLOW
        if (token == EToken_DOUBLE_COLON_EQ) {
            node->isEpsilon = true;
            return node;
        }
        return node;
    }

    static Ptr<parsetree::Node> Parse_Node(Parser &parser) {
        Ptr node = make_unique<parsetree::Node>();
        node->node_ref = Parse_NodeRef(parser);
        node->opt_inherit = Parse_OptInherit(parser);
        return node;
    }

    static Ptr<parsetree::Rule> Parse_Rule(Parser &parser) {
        Ptr node = make_unique<parsetree::Rule>();
        ETokenType token = parser.Peek();
        if (token == EToken_ABSTRACT) {
            node->isAbstract = true;
            parser.Accept(EToken_ABSTRACT);
            node->node = Parse_Node(parser);
            parser.Accept(EToken_SEMICOLON);
        } else {
            // Other case is node
            node->isAbstract = false;
            node->node = Parse_Node(parser);
            parser.Accept(EToken_DOUBLE_COLON_EQ);
            node->child_list = Parse_ChildList(parser);
            parser.Accept(EToken_SEMICOLON);
        }
        return node;
    }

    static Ptr<parsetree::RuleList> Parse_RuleList(Parser &parser) {
        Ptr node = make_unique<parsetree::RuleList>();
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_ID || token == EToken_ABSTRACT) {
            node->isEpsilon = false;
            node->rule = Parse_Rule(parser);
            node->rule_list = Parse_RuleList(parser);
            return node;
        }
        // FOLLOW
        if (token == EToken_EOF) {
            node->isEpsilon = true;
            return node;
        }

        PARSER_PANIC();
    }

    static Ptr<parsetree::Start> Parse_Start(Parser &parser) {
        Ptr node = make_unique<parsetree::Start>();
        node->rule_list = Parse_RuleList(parser);
        parser.Accept(EToken_EOF);
        return node;
    }

    void Parser::Parse(Token *pTokens) {
        m_Ptr = pTokens;
        Parse_Start(*this);
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
        return m_Ptr->line;
    }

    int Parser::CurrCol() {
        if (m_ReachedEof) {
            return -1;
        }
        assert(m_Ptr);
        return m_Ptr->col;
    }
}
