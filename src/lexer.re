#include "lexer.h"

namespace zblast {

static Token NextLine(const char*& YYCURSOR) {
    while (char c = *YYCURSOR++) {
        if (c == '\n') {
            return NextToken(YYCURSOR);
        }
    }
    return { EToken_EOF, {} };
}
static Token NextCloseComment(const char*& YYCURSOR) {
    while (char c = *YYCURSOR++) {
        if (c == '*' && *YYCURSOR == '/') {
            ++YYCURSOR;
            return NextToken(YYCURSOR);
        }
    }
    return { EToken_EOF, {} };
}

Token NextToken(const char*& YYCURSOR) {
    const char* YYMARKER;
    const char* start = YYCURSOR;

    // Skip whitespace
    /*!re2c
        re2c:define:YYCTYPE = "char";
        re2c:yyfill:enable = 0;

        [ \n\r\t]+ { return NextToken(YYCURSOR); }

        // Multi line comment
        "/*" { return NextCloseComment(YYCURSOR); }

        // Single line comment
        "//" { return NextLine(YYCURSOR); }

        // Multi-character token
        "::=" { return {EToken_DOUBLE_COLON_EQ, {start, static_cast<size_t>(YYCURSOR - start)}}; }

        ":"  { return {EToken_COLON, {start, 1}}; }
        ";"  { return {EToken_SEMICOLON, {start, 1}}; }
        "*"  { return {EToken_STAR, {start, 1}}; }
        "|"  { return {EToken_PIPE, {start, 1}}; }
        "["  { return {EToken_L_BRACKET, {start, 1}}; }
        "]"  { return {EToken_R_BRACKET, {start, 1}}; }
        "<"  { return {EToken_L_ANGLE, {start, 1}}; }
        ">"  { return {EToken_R_ANGLE, {start, 1}}; }

        "abstract" { return {EToken_ABSTRACT, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        // A label is an identifier immediately followed by a colon (e.g. "Label:")
        [a-zA-Z_][a-zA-Z0-9_]* ":" {
            YYCURSOR--;
            return {EToken_LABEL, {start, static_cast<size_t>(YYCURSOR - start)}};
        }

        [a-zA-Z_][a-zA-Z0-9_]* {
            return {EToken_ID, {start, static_cast<size_t>(YYCURSOR - start)}};
        }

        // End of input
        "\000" { return {EToken_EOF, {start, 0}}; }

        // Fallback catch-all error rule
        * { return {EToken_ERR, {start, 1}}; }
    */
}
} // zblast

