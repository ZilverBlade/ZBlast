#pragma once

namespace zblast {
    namespace parsetree {
        struct ParseNode;
        struct NodeRef;
        struct OptInherit;
        struct Node;
        struct Iden;
        struct LabeledIden;
        struct VariantQual;
        struct ChildQual;
        struct Child;
        struct ChildList;
        struct OptExpansion;
        struct Rule;
        struct RuleList;
        struct Start;
    }
    class Visitor {
    public:
        [[maybe_unused]] virtual void* Visit(parsetree::NodeRef* node, void* data);
        [[maybe_unused]] virtual void* Visit(parsetree::OptInherit* node, void* data);
        [[maybe_unused]] virtual void* Visit(parsetree::Node* node, void* data);
        [[maybe_unused]] virtual void* Visit(parsetree::Iden* node, void* data);
        [[maybe_unused]] virtual void* Visit(parsetree::LabeledIden* node, void* data);
        [[maybe_unused]] virtual void* Visit(parsetree::VariantQual* node, void* data);
        [[maybe_unused]] virtual void* Visit(parsetree::ChildQual* node, void* data);
        [[maybe_unused]] virtual void* Visit(parsetree::Child* node, void* data);
        [[maybe_unused]] virtual void* Visit(parsetree::ChildList* node, void* data);
        [[maybe_unused]] virtual void* Visit(parsetree::OptExpansion* node, void* data);
        [[maybe_unused]] virtual void* Visit(parsetree::Rule* node, void* data);
        [[maybe_unused]] virtual void* Visit(parsetree::RuleList* node, void* data);
        [[maybe_unused]] virtual void* Visit(parsetree::Start* node, void* data);

    protected:
        [[maybe_unused]] virtual void* VisitChildren(parsetree::ParseNode* node, void* data);
    };
} // namespace zblast
