#include "generator.h"

#include <ctime>
#include <iomanip>

namespace zblast {
    void Generator::Generate(parsetree::Start* program, std::ostream& hpp, std::ostream& cpp, const std::string& fileName) {
        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);   // not thread-safe, see below

        auto WriteHeader = [&](std::ostream& stream, const std::string& fileName) {
            stream << "// " << fileName << R"(
// ********************************************************************************************************
//
// MIT License
// 
// Copyright (c) 2026 ZilverBlade
//
// This file was automatically generated on )" << std::put_time(&tm, "%Y-%m-%d @ %H:%M:%S") << R"( using ZBlast
//
// ********************************************************************************************************
//
// clang-format off
)";
        };

        WriteHeader(hpp, fileName + ".hpp");
        hpp << "#ifndef _ZBLAST_GEN_" << fileName << "_" << "\n";
        hpp << "#define _ZBLAST_GEN_" << fileName << "_ 1" << "\n\n";
        hpp << R"(
// Standard C types
#include <cstdint>
#include <cstddef>

// Standard C++ types
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace ast {

// Aliases, for easy use
template <class T>     
using List = std::vector<T>;
template <class T>     
using Opt = std::optional<T>;
template <class... Rem>
using Variant = std::variant<Rem...>;
using String = std::string;

)";

        WriteHeader(cpp, fileName + ".cpp");
        cpp << "#include \"" << fileName << ".hpp\"" << "\n\n"; 
        cpp << R"(
// Standard C utilities
#include <cassert>

// Standard C++ utilities
#include <stdexcept>

namespace ast {
)";


        Generate(program, hpp, cpp);

        hpp << "\n} // namespace ast\n";
        hpp << "\n#endif // _ZBLAST_GEN_" << fileName << "_" << "\n";
        hpp << "// clang-format on\n";

        cpp << "\n} // namespace ast\n";
        cpp << "// clang-format on\n";
    }
} // namespace zblast