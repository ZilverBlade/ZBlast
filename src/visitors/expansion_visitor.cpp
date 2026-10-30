#pragma once
#include "expansion_visitor.h"
#include "parsetree.h"

namespace zblast {
    namespace visitors {
        void* ExpansionVisitor::Visit(parsetree::Rule* node, void* data) {
            return VisitChildren(node, data);
        }
        void* ExpansionVisitor::Visit(parsetree::OptExpansion* node, void* data) {
            return VisitChildren(node, data);
        }
        void* ExpansionVisitor::Visit(parsetree::ChildList* node, void* data) {
            return VisitChildren(node, data);
        }
        void* ExpansionVisitor::Visit(parsetree::Child* node, void* data) {
            return VisitChildren(node, data);
        }
    } // namespace visitors
} // namespace zblast
