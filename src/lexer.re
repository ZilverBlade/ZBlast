#include "lexer.h"

namespace zblast {

static Token NextLine(const char*& YYCURSOR, LexerState* state) {
    while (char c = *YYCURSOR++) {
        if (c == '\n') {
            return NextToken(YYCURSOR, state);
        }
    }
    return { EToken_EOF, {} };
}
static Token NextCloseComment(const char*& YYCURSOR, LexerState* state) {
    while (char c = *YYCURSOR++) {
        if (c != '\r') {
            ++state->col;
        }
        if (c == '\n') {
            ++state->line;
            state->col = 1;
        } else if (c == '*' && *YYCURSOR == '/') {
            ++YYCURSOR;
            return NextToken(YYCURSOR, state);
        }
    }
    return { EToken_EOF, {} };
}

static Token NextToken_Impl(const char*& YYCURSOR, LexerState* state) {
    const char* YYMARKER;
    const char* start = YYCURSOR;

    // Skip whitespace
    /*!re2c
        re2c:define:YYCTYPE = "char";
        re2c:yyfill:enable = 0;

        [ \t]+ {
            state->col += YYCURSOR - start;
            return NextToken(YYCURSOR, state);
        }
        "\r\n" { // CRLF
            ++state->line;
            state->col = 1;
            return NextToken(YYCURSOR, state);
        }
        "\n" { // LF
            ++state->line;
            state->col = 1;
            return NextToken(YYCURSOR, state);
        }
        // Single line comment
        "/*" { return NextCloseComment(YYCURSOR, state); }
        "//" {
            ++state->line;
            state->col = 1;
            return NextLine(YYCURSOR, state);
        }

        // Multi-character token
        "::=" { return {EToken_DOUBLE_COLON_EQ, {start, static_cast<size_t>(YYCURSOR - start)}, state->line, state->col}; }

        ":"  { return {EToken_COLON, {start, 1}, state->line, state->col}; }
        ";"  { return {EToken_SEMICOLON, {start, 1}, state->line, state->col}; }
        "*"  { return {EToken_STAR, {start, 1}, state->line, state->col}; }
        "|"  { return {EToken_PIPE, {start, 1}, state->line, state->col}; }
        "["  { return {EToken_L_BRACKET, {start, 1}, state->line, state->col}; }
        "]"  { return {EToken_R_BRACKET, {start, 1}, state->line, state->col}; }
        "<"  { return {EToken_L_ANGLE, {start, 1}, state->line, state->col}; }
        ">"  { return {EToken_R_ANGLE, {start, 1}, state->line, state->col}; }

        "abstract" { return {EToken_ABSTRACT, {start, YYCURSOR - start}, state->line, state->col}; }
        // A label is an identifier immediately followed by a colon (e.g. "Label:")
        [a-zA-Z_][a-zA-Z0-9_]* ":" {
            YYCURSOR--;
            return {EToken_LABEL, {start, static_cast<size_t>(YYCURSOR - start)}, state->line, state->col};
        }

        [a-zA-Z_][a-zA-Z0-9_]* {
            return {EToken_ID, {start, static_cast<size_t>(YYCURSOR - start)}, state->line, state->col};
        }

        // End of input
        "\000" { return {EToken_EOF, {start, 0}, state->line, state->col}; }

        // Fallback catch-all error rule
        * { return {EToken_ERR, {start, 1}, state->line, state->col}; }
    */
}

Token NextToken(const char*& YYCURSOR, LexerState* state) {
   Token tok = NextToken_Impl(YYCURSOR, state);
   state->col += tok.text.length();
   return tok;
}


} // zblast

