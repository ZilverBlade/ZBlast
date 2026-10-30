#pragma once
#include <ostream>

namespace zblast {
    namespace parsetree {
        struct AstNode;
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
    
    class Generator {
    public:
        void Generate(parsetree::Start* node, std::ostream& hpp, std::ostream& cpp, const std::string& fileName);

    protected:
        virtual void Generate(parsetree::Start* node, std::ostream& hpp, std::ostream& cpp) = 0;

    };
} // namespace zblast
