#include <cstring>
#include <iostream>
#include <vector>

#include "lexer.h"
#include "parser.h"
#include "parsetree.h"

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
 child_qual -> "*"              # List
 child_qual -> "|" child        # Variant
 child_qual -> \epsilon

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
        tokens.push_back(tok);
        if (tok.type == zblast::EToken_EOF) {
            isEnd = true;
        }
        tokens.push_back(tok);
    }
    return tokens;
}

int main(int argc, char **argv) {
    try {
        if (argc < 4) {
            throw std::runtime_error("Insufficient arguments provided, usage: [-t/-p/-g] <source> <target>");
        }
        bool doTokenize = strcmp(argv[1], "-t") == 0;
        bool doEmitParseTree = strcmp(argv[1], "-p") == 0;
        bool doGeneration = strcmp(argv[1], "-g") == 0;

        const char *srcFile = argv[2];
        const char *dstFile = argv[3];

        FILE *srcTextF = fopen(srcFile, "r");
        if (!srcTextF) {
            char errBuf[128];
            snprintf(errBuf, 128, "Failed to open source file '%s'!", srcFile);
            throw std::runtime_error(errBuf);
        }
        FILE *dstTextF = fopen(dstFile, "w+");
        if (!dstTextF) {
            char errBuf[128];
            snprintf(errBuf, 128, "Failed to open write to target file '%s'!", dstFile);
            throw std::runtime_error(errBuf);
        }

        if (doTokenize || doEmitParseTree || doGeneration) {
            std::string text;
            fseek(srcTextF, 0, SEEK_SET);
            fseek(srcTextF, 0, SEEK_END);
            long int fsize = ftell(srcTextF);
            text.resize(fsize);
            fseek(srcTextF, 0, SEEK_SET);
            fread(text.data(), text.size(), text.size(), srcTextF);
            fclose(srcTextF);
            auto tokens = Tokenize(text.c_str());
            if (doTokenize) {
                for (auto tok: tokens) {
                    fputs(zblast::ETokenType_ToString(tok.type), dstTextF);
                    if (tok.type == zblast::EToken_ID || tok.type == zblast::EToken_LABEL || tok.type ==
                        zblast::EToken_ERR) {
                        fprintf(dstTextF, " \"%.*s\"", static_cast<int>(tok.text.length()), tok.text.data());
                    }
                    if (tok.type == zblast::EToken_ERR) {
                        fprintf(dstTextF, " @ %d:%d", tok.line, tok.col);
                    }
                    fputc('\n', dstTextF);
                }
            } else if (doEmitParseTree || doGeneration) {
                if (doEmitParseTree) {
                    zblast::Parser parser;
                    auto prog = parser.Parse(tokens.data());
                    std::stringstream ss;
                    zblast::parsetree::PrintTree(prog.get(), ss);
                    fputs(ss.str().c_str(), dstTextF);
                } else if (doGeneration) {
                    // TODO
                    std::cerr << "Not implemented!" << std::endl;
                }
            }
            fclose(dstTextF);
        }
    } catch (std::runtime_error &ex) {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
