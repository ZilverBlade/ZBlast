#pragma once
#include "declaration_visitor.h"
#include "parsetree.h"

namespace zblast {
    namespace visitors {
        void* DeclarationVisitor::Visit(parsetree::Rule* node, void* data) {
            Declaration decl;
            decl.isAbstract = node->IsAbstract;
            decl.className = node->node->node_ref->ID;
            if (!node->node->opt_inherit->IsEpsilon) {
                decl.classInherit = node->node->opt_inherit->node_ref->ID;
            }
            else {
                decl.classInherit = std::nullopt;
            }
            decl.declaration = node->node.get();
            m_Declarations.push_back(decl);
            return VisitChildren(node, data);
        }
    } // namespace visitors
} // namespace zblast