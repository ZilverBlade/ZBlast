#pragma once
#include "visitor.h"
#include <string>
#include <vector>
#include <optional>

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
            EExpansion_Normal,
            EExpansion_List,
            EExpansion_Opt,
            EExpansion_Variant
        };
        struct ExpansionMember {
            EExpansionKind kind;                    
            std::vector<MemberSpecifier> specifiers; // Always at least one, only >1 if a variant.
        };
        struct Expansion {
            std::string className;                // Name of the node
            parsetree::Node* declaration;         // Node that declares it
            std::vector<ExpansionMember> members; // Right hand side of the rule
        };
        class ExpansionVisitor : public Visitor {
        public:
            void* Visit(parsetree::Rule* node, void* data);
            void* Visit(parsetree::OptExpansion* node, void* data);
            void* Visit(parsetree::ChildList* node, void* data);
            void* Visit(parsetree::Child* node, void* data);

            const std::vector<Expansion>& GetExpansions() const {
                return m_Expansions;
            }
        private:
            std::vector<Expansion> m_Expansions;
        };
    } // namespace visitors
} // namespace zblast