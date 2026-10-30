#include "parser.h"

#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <memory>

using Str = std::string;

template<typename T>
using Ptr = std::unique_ptr<T>;

using std::make_unique;

namespace zblast {
    namespace parsetree {
        struct ParseNode {
        };

        struct NodeRef : ParseNode {
            Str ID;
        };

        struct OptInherit {
            bool isEpsilon;
            Ptr<NodeRef> node_ref;
        };

        struct Node : ParseNode {
            Ptr<NodeRef> node_ref;
            Ptr<OptInherit> opt_inherit;
        };

        struct NamedNode : ParseNode {
            bool hasLabel;
            Str LABEL;
            Str ID;
        };

        struct Child;

        struct ChildQual : ParseNode {
            enum {
                Mode_List,
                Mode_Variant,
                IsEpsilon,
            } Mode;

            Ptr<Child> child;
        };

        struct Child : ParseNode {
            enum {
                Mode_Literal,
                Mode_Optional,
                Mode_Qualifier,
            } Mode;

            Str ID;
            Ptr<NamedNode> named_node;
            Ptr<ChildQual> child_qual;
        };

        struct ChildList : ParseNode {
            bool isEpsilon;
            Ptr<Child> child;
            Ptr<ChildList> child_list;
        };

        struct Rule : ParseNode {
            Ptr<Node> node;
            Ptr<ChildList> child_list;
        };

        struct RuleList : ParseNode {
            bool isEpsilon;
            Ptr<Rule> rule;
            Ptr<RuleList> rule_list;
        };

        struct Start : ParseNode {
            Ptr<RuleList> rule_list;
        };
    }

#define PARSER_PANIC() do {\
    char errBuf[128];\
    snprintf(errBuf, sizeof(errBuf), "Failed to match parse token %s in rule %s! @ %d:%d", ETokenType_ToString(parser.Peek()), __func__, parser.CurrLine(), parser.CurrCol());\
    throw std::runtime_error(errBuf);\
} while (false)

    static Ptr<parsetree::NamedNode> Parse_NamedNode(Parser &parser) {
        Ptr node = make_unique<parsetree::NamedNode>();
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_LABEL) {
            node->hasLabel = true;
            node->LABEL = parser.Accept(EToken_LABEL).text;
            parser.Accept(EToken_COLON);
            node->ID = parser.Accept(EToken_ID).text;
            return node;
        }

        if (token == EToken_ID) {
            node->hasLabel = false;
            node->ID = parser.Accept(EToken_ID).text;
            return node;
        }

        PARSER_PANIC();
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
        if (token == EToken_R_BRACKET || token == EToken_STAR) {
            node->Mode = parsetree::ChildQual::IsEpsilon;
            return node;
        }

        PARSER_PANIC();
    }

    static Ptr<parsetree::Child> Parse_Child(Parser &parser) {
        Ptr node = make_unique<parsetree::Child>();
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_L_ANGLE) {
            node->Mode = parsetree::Child::Mode_Literal;
            parser.Accept(EToken_L_ANGLE);
            node->ID = parser.Accept(EToken_ID).text;
            parser.Accept(EToken_R_ANGLE);
            return node;
        }
        if (token == EToken_L_BRACKET) {
            node->Mode = parsetree::Child::Mode_Optional;
            parser.Accept(EToken_L_BRACKET);
            node->named_node = Parse_NamedNode(parser);
            parser.Accept(EToken_R_BRACKET);
            return node;
        }
        if (token == EToken_LABEL || token == EToken_ID) {
            node->Mode = parsetree::Child::Mode_Qualifier;
            node->named_node = Parse_NamedNode(parser);
            node->child_qual = Parse_ChildQual(parser);
            return node;
        }

        PARSER_PANIC();
    }

    static Ptr<parsetree::ChildList> Parse_ChildList(Parser &parser) {
        Ptr node = make_unique<parsetree::ChildList>();
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_ID) {
            node->isEpsilon = false;
            node->child = Parse_Child(parser);
            node->child_list = Parse_ChildList(parser);
            return node;
        }
        // FOLLOW
        if (token == EToken_EOF) {
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
        node->node = Parse_Node(parser);
        parser.Accept(EToken_DOUBLE_COLON_EQ);
        node->child_list = Parse_ChildList(parser);
        parser.Accept(EToken_SEMICOLON);
        return node;
    }

    static Ptr<parsetree::RuleList> Parse_RuleList(Parser &parser) {
        Ptr node = make_unique<parsetree::RuleList>();
        ETokenType token = parser.Peek();
        // FIRST
        if (token == EToken_ID) {
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
            snprintf(errBuf, sizeof(errBuf), "Expected token %s but got %s!", ETokenType_ToString(expected),
                     ETokenType_ToString(actual));
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
