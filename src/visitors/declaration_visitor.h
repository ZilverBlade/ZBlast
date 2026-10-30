#pragma once
#include "visitor.h"
#include <string>
#include <vector>
#include <optional>

namespace zblast {
    namespace visitors {
        struct Declaration {
            bool isAbstract;
            std::string className;
            parsetree::Node* declaration;
            parsetree::Rule* rule;
            std::optional<std::string> classInherit;
        };
        class DeclarationVisitor : public Visitor {
        public:
            std::vector<Declaration> CollectDeclarations(parsetree::Start* program);

        protected:
            void* Visit(parsetree::Rule* node, void* data) override;

        private:
            std::vector<Declaration> m_Declarations;
        };
    } // namespace visitors
} // namespace zblast