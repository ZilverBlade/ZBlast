#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <optional>
#include <memory>
#include <vector>
#include <filesystem>

#include "generator.h"
#include "lexer.h"
#include "parser.h"
#include "parsetree.h"
#include "util.h"
#include "generators/ast_code_gen.h"
#include "generators/visitor_code_gen.h"
#include "visitors/error_visitor.h"

/*

 start -> rule_list EOF
 rule_list -> rule rule_list
 rule_list -> \epsilon

 rule -> node opt_expansion ";"
 rule -> "abstract" node ";"
 opt_expansion -> "::=" child_list
 opt_expansion -> \epsilon
 node -> node_ref opt_inherit

 node_ref -> ID
 opt_inherit -> ":" node_ref
 opt_inherit -> \epsilon

 child_list -> child child_list
 child_list -> \epsilon

 labeled_iden -> LABEL ":" iden
 labeled_iden -> iden
 iden -> ID
 iden -> "<" ID ">"
 child -> "[" labeled_iden "]"    # Optional
 child -> labeled_iden child_qual # Literal/Identifier
 child_qual -> "*"                # List
 child_qual -> variant_qual       # Variant
 child_qual -> \epsilon
 variant_qual -> "|" labeled_iden variant_qual
 variant_qual -> \epsilon

*/


static std::vector<zblast::Token> Tokenize(const char* src) {
    std::vector<zblast::Token> tokens;
    bool isEnd = false;
    const char* cursor = src;
    int l = 1, c = 1;
    while (!isEnd) {
        const char* start = cursor;
        zblast::Token tok = zblast::NextToken(cursor);
        zblast::AdvanceLocation(l, c, start, cursor);
        tok.colEnd = c;
        tok.lineEnd = l;
        tok.lineStart = l;
        tok.colStart = tok.colEnd - static_cast<int>(tok.text.length());
        if (tok.type == zblast::EToken_EOF) {
            isEnd = true;
        }
        tokens.push_back(tok);
    }
    return tokens;
}

enum EGenerationMode {
    EGeneration_Ast,
    EGeneration_Visitor,
};
//
//struct CliOption {
//    char shortName = '\0';
//    std::string longName;
//    std::vector<std::string> arguments;
//    std::string description;
//};
////
//static std::vector<CliOption> g_Options = {
//    { 'o', "output", { "FILE..." }, "Write result to FILES (default: stdout). Some options may require two outputs." },
//    { 'v', "verbose", {}, "Increase verbosity" },
//    { 'h', "help", {}, "Show this help and exit" },
//    { {}, "version", {}, "Show version and exit" },
//};
//
//static void PrintRow(std::ostream& out, const std::string& left,
//    const std::string& desc, size_t width) {
//    out << "  " << std::left << std::setw((int)width) << left << "  " << desc << "\n";
//}
//
//static void PrintUsage(std::ostream& out) {
//    std::vector<std::string> lefts;
//    size_t width = std::string_view("<input>").length();
//    for (auto& o : g_Options) {
//        std::string text = o.shortName ? std::string("-") + o.shortName + ", " : "    ";
//        text += "--" + o.longName;
//        for (auto& a : o.arguments) text += " " + a;
//        width = std::max(width, text.size());
//        lefts.push_back(std::move(text));
//    }
//
//    out << "-- ZBlast Generator --\n\n"
//        << "Usage:\n  zblast [options] <input>\n\n"
//        << "Arguments:\n";
//    PrintRow(out, "<input>", "ZBlast (.zast) file to process", width);
//
//    out << "\nOptions:\n";
//    for (size_t i = 0; i < g_Options.size(); ++i) {
//        PrintRow(out, lefts[i], g_Options[i].description, width);
//    }
//
//    out << R"(Example:
//  zblast --validate grammar.zast                                # Validate the output, writes to stderr
//  zblast -o grammar.hpp grammar.cpp --generate-ast grammar.zast # Generate the ast, writes to grammar.hpp (header) and grammar.cpp (impl)
//)";
//}

int main(int argc, char** argv) {
    try {
        if (argc < 4) {
            throw std::runtime_error("Insufficient arguments provided, usage: [-avc] <source> <target>");
        }
        EGenerationMode mode;
        if (strcmp(argv[1], "-a") == 0) {
            mode = EGeneration_Ast;
        } else if (strcmp(argv[1], "-v") == 0) {
            mode = EGeneration_Visitor;
        } else {
            throw std::runtime_error(zblast::util::Format("Invalid argument \"%s\"", argv[1]));
        }

        std::string srcFile = argv[2];
        std::string dstFile = argv[3];

        // Step 0: reading
        std::ifstream in(srcFile, std::ios::in);
        if (!in) {
            throw std::runtime_error(zblast::util::Format("Failed to open source file '%s'!", srcFile.c_str()));
        }
        std::ostringstream ss;
        ss << in.rdbuf();
        std::string text = ss.str();
        // Step 1: scanning
        auto tokens = Tokenize(text.c_str());


        // Step 2: parsing
        zblast::Parser parser;
        auto prog = parser.Parse(tokens.data());


        // Step 3: Check the AST
        std::string srcFileAbsolute = std::filesystem::absolute(srcFile).string();
        bool hasError = false;
        const auto& errors = zblast::visitors::ErrorVisitor().CollectErrors(prog.get());
        for (auto& error : errors) {
            std::cerr << error.PrettyPrint(srcFileAbsolute, zblast::visitors::EErrorFormat_MSVC) << std::endl;
            hasError |= zblast::visitors::k_ErrorDefs[error.code].severity == zblast::visitors::ESeverity_Error;
        }

        if (hasError) {
            return EXIT_FAILURE;
        }

        // Step 4: open the output file
        size_t pos = std::min(dstFile.find_last_of('\\'), dstFile.find_last_of('/'));
        std::string dstFileNameOnly = pos == std::string::npos ? dstFile : dstFile.substr(pos + 1);
        std::string dstFileHpp = dstFile + ".hpp";
        std::string dstFileCpp = dstFile + ".cpp";

        FILE* dstTextFHpp = fopen(dstFileHpp.c_str(), "w+");
        if (!dstTextFHpp) {
            throw std::runtime_error(zblast::util::Format("Failed to open write to target file '%s'!", dstFileHpp.c_str()));
        }
        FILE* dstTextFCpp = fopen(dstFileCpp.c_str(), "w+");
        if (!dstTextFCpp) {
            throw std::runtime_error(zblast::util::Format("Failed to open write to target file '%s'!", dstFileCpp.c_str()));
        }

        // Step 5: finally GENERATE

        std::unique_ptr<zblast::Generator> generator;
        switch (mode) {
        case EGeneration_Ast: {
            std::cout << "Generating .cpp/.hpp files for AST..." << std::endl;
            generator = std::make_unique<zblast::generators::AstCodeGen>();
            break;
        }
        case EGeneration_Visitor: {
            std::cout << "Generating .cpp/.hpp files for Visitors..." << std::endl;
            generator = std::make_unique<zblast::generators::VisitorCodeGen>();
            break;
        }
        default: throw std::runtime_error("BAD CASE!!! This should never happen!");
        }

        zblast::util::FileBuf hppBuf(dstTextFHpp);
        zblast::util::FileBuf cppBuf(dstTextFCpp);

        std::ostream hppStream(&hppBuf);
        std::ostream cppStream(&cppBuf);

        generator->Generate(prog.get(), hppStream, cppStream, dstFileNameOnly);

        fclose(dstTextFHpp);
        fclose(dstTextFCpp);
    } catch (std::runtime_error& ex) {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
