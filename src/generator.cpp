#include "generator.h"

#include <ctime>
#include <iomanip>

namespace zblast {
    void Generator::Generate(parsetree::Start* program, std::ostream& hpp, std::ostream& cpp, const std::string& stem) {
        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);   // not thread-safe, see below

        std::string outputStem = this->GatherOutputStem(stem);
        auto WriteHeader = [&](std::ostream& stream, bool isHpp) {
            stream << "// " << outputStem << (isHpp ? ".hpp" : ".cpp") << R"(
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

        WriteHeader(hpp, true);
        hpp << "#ifndef _ZBLAST_GEN_" << outputStem << "_" << "\n";
        hpp << "#define _ZBLAST_GEN_" << outputStem << "_ 1" << "\n\n";
        hpp << R"(
// If you need to export the symbols to a DLL (e.g. windows __declspec(dllexport/dllimport))
// You may define "ZBLAST_API" to define the export symbols  
#ifndef ZBLAST_API
#define ZBLAST_API
#endif

// Standard C types
#include <cstdint>
#include <cstddef>

// Standard C++ types
#include <string>
#include <optional>
#include <vector>
#include <variant>

namespace ast {

// Aliases, for easy use
template <class T>     
using List = std::vector<T>;
template <class T>     
using Opt = std::optional<T>;
using std::nullopt;
template <class... Rem>
using Variant = std::variant<Rem...>;
using String = std::string;
)";

        WriteHeader(cpp, false);
        cpp << "#include \"" << outputStem << ".hpp\"" << "\n\n";
        cpp << AdditionalIncludes(stem);
        cpp << R"(
// Standard C utilities
#include <cassert>

// Standard C++ utilities
#include <functional>
#include <stdexcept>

namespace ast {
)";


        Generate(program, hpp, cpp);

        hpp << "\n} // namespace ast\n";
        hpp << "\n#endif // _ZBLAST_GEN_" << outputStem << "_" << "\n";
        hpp << "// clang-format on\n";

        cpp << "\n} // namespace ast\n";
        cpp << "// clang-format on\n";
    }

    std::string Generator::AdditionalIncludes(const std::string& stem) { return ""; }
} // namespace zblast