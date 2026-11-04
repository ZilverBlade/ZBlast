#pragma once
#include "../error.h"
#include "visitor.h"
#include <string>
#include <vector>

namespace zblast {
    namespace visitors {
        class ErrorVisitor : public Visitor {
        public:
            std::vector<Error> CollectErrors(parsetree::Start* program);

        protected:
            void* Visit(parsetree::Start* node, void* data) override;

        private:

            std::vector<Error> m_Errors;
        };
    } // namespace visitors
} // namespace zblast