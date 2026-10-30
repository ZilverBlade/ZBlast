#include "expansion_visitor.h"
#include "parsetree.h"

namespace zblast {
    namespace visitors {
        std::vector<Expansion> ExpansionVisitor::CollectExpansions(parsetree::Start* program) {
            program->Accept(this, nullptr);
            return std::move(m_Expansions);
        }

        void* ExpansionVisitor::Visit(parsetree::Rule* node, void* data) {
            Expansion expansion;
            expansion.rule = node;
            expansion.className = node->node->node_ref->ID;
            VisitChildren(node, &expansion);
            m_Expansions.push_back(expansion);
            return data;
        }
        void* ExpansionVisitor::Visit(parsetree::ChildList* node, void* data) {
            if (!node->IsEpsilon) {
                Expansion& expansion = *reinterpret_cast<Expansion*>(data);
                ExpansionMember member;
                node->child->Accept(this, &member);
                expansion.members.emplace_back(std::move(member));
                return node->child_list->Accept(this, data);
            }
            return VisitChildren(node, data);
        }
        void* ExpansionVisitor::Visit(parsetree::Child* node, void* data) {
            ExpansionMember& expansionMember = *reinterpret_cast<ExpansionMember*>(data);
            if (node->Mode == parsetree::Child::Mode_Optional) {
                expansionMember.kind = EExpansion_Opt;
            } else if (node->Mode == parsetree::Child::Mode_IdentifierWQualifier) {
                if (node->child_qual->Mode == parsetree::ChildQual::Mode_List) {
                    expansionMember.kind = EExpansion_List;
                } else if (node->child_qual->Mode == parsetree::ChildQual::Mode_Variant) {
                    expansionMember.kind = EExpansion_Variant;
                } else if (node->child_qual->Mode == parsetree::ChildQual::IsEpsilon) {
                    expansionMember.kind = EExpansion_Normal;
                } 
            }
            MemberSpecifier specifier;
            node->labeled_iden->Accept(this, &specifier);
            expansionMember.specifiers.emplace_back(std::move(specifier));
            
            // Grow qualifiers
            if (expansionMember.kind == EExpansion_Variant) {
                node->child_qual->variant_qual->Accept(this, &expansionMember);
            } 

            return data;
        }
        void* ExpansionVisitor::Visit(parsetree::LabeledIden* node, void* data) {
            MemberSpecifier& specifier = *reinterpret_cast<MemberSpecifier*>(data);
            node->iden->Accept(this, &specifier);
            specifier.idenRef = node;
            if (!node->HasLabel) {
                specifier.label = specifier.nodeName;
            } else {
                specifier.label = node->LABEL;
            }
            return data;
        }
        void* ExpansionVisitor::Visit(parsetree::Iden* node, void* data) {
            MemberSpecifier& specifier = *reinterpret_cast<MemberSpecifier*>(data);
            specifier.kind = node->IsLiteral ? EMember_Literal : EMember_Identifier;
            specifier.nodeName = node->ID;
            specifier.label = node->ID;
            return VisitChildren(node, data);
        }
        void* ExpansionVisitor::Visit(parsetree::VariantQual* node, void* data) {
            ExpansionMember& expansionMember = *reinterpret_cast<ExpansionMember*>(data);
            if (!node->IsEpsilon) {
                MemberSpecifier specifier;
                node->labeled_iden->Accept(this, &specifier);
                expansionMember.specifiers.emplace_back(std::move(specifier));
                node->variant_qual->Accept(this, &expansionMember);
            }
            return data;
        }
    } // namespace visitors
} // namespace zblast
