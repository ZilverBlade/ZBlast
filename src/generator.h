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
        virtual std::string GatherOutputStem(const std::string& fileStem) = 0;
        void Generate(parsetree::Start* node, std::ostream& hpp, std::ostream& cpp, const std::string& stem);

    protected:
        virtual std::string AdditionalIncludes(const std::string& stem);
        virtual void Generate(parsetree::Start* node, std::ostream& hpp, std::ostream& cpp) = 0;

    };
} // namespace zblast
