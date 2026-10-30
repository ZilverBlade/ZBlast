#include "visitor.h"
#include "parsetree.h"

namespace zblast {
    void* Visitor::Visit(parsetree::NodeRef* node, void* data) { return VisitChildren(node, data); }
    void* Visitor::Visit(parsetree::OptInherit* node, void* data) { return VisitChildren(node, data); }
    void* Visitor::Visit(parsetree::Node* node, void* data) { return VisitChildren(node, data); }
    void* Visitor::Visit(parsetree::Iden* node, void* data) { return VisitChildren(node, data); }
    void* Visitor::Visit(parsetree::LabeledIden* node, void* data) { return VisitChildren(node, data); }
    void* Visitor::Visit(parsetree::ChildQual* node, void* data) { return VisitChildren(node, data); }
    void* Visitor::Visit(parsetree::Child* node, void* data) { return VisitChildren(node, data); }
    void* Visitor::Visit(parsetree::ChildList* node, void* data) { return VisitChildren(node, data); }
    void* Visitor::Visit(parsetree::OptExpansion* node, void* data) { return VisitChildren(node, data); }
    void* Visitor::Visit(parsetree::Rule* node, void* data) { return VisitChildren(node, data); }
    void* Visitor::Visit(parsetree::RuleList* node, void* data) { return VisitChildren(node, data); }
    void* Visitor::Visit(parsetree::Start* node, void* data) { return VisitChildren(node, data); }
    void* Visitor::VisitChildren(parsetree::ParseNode* node, void* data) {
        for (parsetree::ParseNode* child : node->Children()) {
            child->Accept(this, data);
        }
        return data;
    }
} // namespace zblast
