#pragma once
#include "generator.h"

namespace zblast {
    namespace generators {        
        class AstCodeGen : public Generator {
        public:
            void Generate(parsetree::Start* program, std::ostream& hpp, std::ostream& cpp) override;    

        };
    } // namespace generators
} // namespace zblast