#pragma once
#include "generator.h"

namespace zblast {
    namespace generators {        
        class VisitorCodeGen : public Generator {
        protected:
            void Generate(parsetree::Start* program, std::ostream& hpp, std::ostream& cpp) override;    

            std::string AdditionalIncludes(const std::string &fileName) override;
        };
    } // namespace generators
} // namespace zblast