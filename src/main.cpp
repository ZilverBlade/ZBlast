#include <iostream>
#include "lexer.h"

/*

 start -> rule_list EOF
 rule_list -> rule rule_list
 rule_list -> \epsilon

 rule -> node "::=" child_list ";"
 node -> node_ref opt_inherit

 node_ref -> ID
 opt_inherit -> ":" node_ref
 opt_inherit -> \epsilon

 child_list -> child child_list
 child_list -> \epsilon

 named_node -> LABEL ":" ID
 named_node -> ID
 child -> "<" ID ">"            # Literal
 child -> "[" named_node "]"    # Optional
 child -> named_node child_qual
 child_qual -> "*"              # List
 child_qual -> "|" child        # Variant
 child_qual -> \epsilon

*/

int main() {
    const char *src = R"(
        abstract Stmt;
        LetDecl : Stmt ::= <ID>; // let x;
        ConstDecl : Stmt ::= <ID> [Value:<INT_LITERAL>]; // const x: 0;
        VarDeclStmt : Stmt ::= /* variant */ LetDecl | ConstDecl;
    )";

    bool isEnd = false;
    const char *cursor = src;
    int l = 1, c = 1;
    while (!isEnd) {
        const char *start = cursor;
        zblast::Token tok = zblast::NextToken(cursor);
        zblast::AdvanceLocation(l, c, start, cursor);
        tok.line = l;
        tok.col = c;
        if (tok.type == zblast::EToken_EOF) {
            isEnd = true;
        }

        std::cout << zblast::ETokenType_ToString(tok.type);
        if (tok.type == zblast::EToken_ID || tok.type == zblast::EToken_LABEL || tok.type == zblast::EToken_ERR) {
            std::cout << " \"" << tok.text << "\"";
        }
        if (tok.type == zblast::EToken_ERR) {
            std::cout << " @ " << tok.line << ":" << tok.col;
        }
        std::cout << "\n";
    }
    return 0;
}
