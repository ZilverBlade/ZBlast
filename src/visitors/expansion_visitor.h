#pragma once
#include "visitor.h"
#include <string>
#include <vector>

namespace zblast {
    namespace visitors {
        enum EMemberKind {
            EMember_Identifier,
            EMember_Literal,
        };
        struct MemberSpecifier {
            EMemberKind kind;
            std::string label;    // for function name
            std::string nodeName; // for determining which class
        };
        enum EExpansionKind {
            // Stmt -> get_Stmt
            // Branch:Stmt -> get_Branch
            EExpansion_Normal,

            // Stmt* -> get_Stmts, get_Stmt(size_t), get_StmtCount()
            // Arg:Expr* -> get_Args, get_Arg(size_t), get_ArgCount()
            EExpansion_List,

            // [Stmt] -> get_Stmt, has_Stmt()
            // [True:Stmt] -> get_True, has_True()
            EExpansion_Opt,

            // VarInit | ConstInit -> get_VarInit, get_ConstInit(), has_VarInit, has_ConstInit()
            EExpansion_Variant
        };
        struct ExpansionMember {
            EExpansionKind kind;                    
            std::vector<MemberSpecifier> specifiers; // Always at least one, only >1 if a variant.
        };
        struct Expansion {
            std::string className;                // Name of the node
            parsetree::Rule* rule;                // Node that declares it
            std::vector<ExpansionMember> members; // Right hand side of the rule
        };
        class ExpansionVisitor : public Visitor {
        public:
            void* Visit(parsetree::Rule* node, void* data);
            void* Visit(parsetree::ChildList* node, void* data);
            void* Visit(parsetree::Child* node, void* data);
            void* Visit(parsetree::LabeledIden* node, void* data);
            void* Visit(parsetree::Iden* node, void* data);
            void* Visit(parsetree::VariantQual* node, void* data);

            const std::vector<Expansion>& GetExpansions() const {
                return m_Expansions;
            }
        private:
            std::vector<Expansion> m_Expansions;
        };
    } // namespace visitors
} // namespace zblast