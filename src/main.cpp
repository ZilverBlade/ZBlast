#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

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


static std::vector<zblast::Token> Tokenize(const char *src) {
    std::vector<zblast::Token> tokens;
    bool isEnd = false;
    const char *cursor = src;
    int l = 1, c = 1;
    while (!isEnd) {
        const char *start = cursor;
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

int main(int argc, char **argv) {
    try {
        if (argc < 4) {
            throw std::runtime_error("Insufficient arguments provided, usage: [-av] <source> <target>");
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
        FILE *srcTextF = fopen(srcFile.c_str(), "r");
        if (!srcTextF) {
            char errBuf[128];
            snprintf(errBuf, 128, "Failed to open source file '%s'!", srcFile.c_str());
            throw std::runtime_error(errBuf);
        }

        // Step 1: scanning
        std::string text;
        fseek(srcTextF, 0, SEEK_SET);
        fseek(srcTextF, 0, SEEK_END);
        long int fsize = ftell(srcTextF);
        text.resize(fsize);
        fseek(srcTextF, 0, SEEK_SET);
        fread(text.data(), text.size(), text.size(), srcTextF);
        fclose(srcTextF);
        auto tokens = Tokenize(text.c_str());


        // Step 2: parsing
        zblast::Parser parser;
        auto prog = parser.Parse(tokens.data());
            
        
        // Step 3: Check the AST

        bool hasError = false;
        const auto& errors = zblast::visitors::ErrorVisitor().CollectErrors(prog.get());
        for (auto& error : errors) {
            std::cerr << error.PrettyPrint() << std::endl;
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
    } catch (std::runtime_error &ex) {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
