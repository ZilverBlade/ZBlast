// re2c
#include "lexer.h"
#include <cassert>
#include <cctype>

namespace zblast {

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcomment"
#endif


static Token NextLine(const char*& YYCURSOR) {
    while (char c = *YYCURSOR++) {
        if (c == '\n') {
            return NextToken(YYCURSOR);
        }
    }
    return { EToken_EOF, {} };
}
static Token NextCloseComment(const char*& YYCURSOR) {
    int openCommentCount = 1;
    while (char c = *YYCURSOR++) {
        if (c == '/' && *YYCURSOR == '*') {
            ++openCommentCount;
        } else if (c == '*' && *YYCURSOR == '/') {
            --openCommentCount;
        } 
        if (openCommentCount == 0) {
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
        // A label is an identifier immediately followed by a colon e.g. "Label:"

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
    assert(false && "unreachable");
}

static char AdvanceCppEscaped(const char*& YYCURSOR) {
    while (char c = *YYCURSOR++) {
        // ESCAPE 1: R"<text>()<text>"
        if (c == 'R' && *YYCURSOR == '"') {
            std::string rqouteClosingText = "";
            int flagRQuote = 0;
            while (char xparen = *++YYCURSOR) {
                if (xparen == '\000') {
                    return false;
                }
                // invalid characters in R" quote, c++ syntax error
                if (xparen == ' ' || xparen == '\r' || xparen == '\n' || xparen == '\t') continue;
                // Found the opening tag
                if (xparen == '(') {
                    flagRQuote = 1;
                    break;
                // " before the opening tag is invalid C++
                } else if (xparen == '"') {
                    flagRQuote = 0;
                    rqouteClosingText.clear();
                    break;
                } else {
                    // keep track of everything between R" and (
                    rqouteClosingText += xparen; 
                }
            }
            if (!flagRQuote) {
                continue;
            }
            while (char xparen = *++YYCURSOR) {
                if (xparen == '\000') {
                    return false;
                }
                // Found the closing tag
                if (xparen == ')') {
                    bool closedQuote = true;
                    for (char expected : rqouteClosingText) {
                        if (expected != *++YYCURSOR) { 
                            closedQuote = false;
                            break;
                        }
                    }
                    if (closedQuote) {
                        if (*++YYCURSOR == '"') {
                            // success!!! we finally finished escaping it!
                            flagRQuote = 0;
                            ++YYCURSOR;
                            break;
                        } else {
                            // Invalid quote!!
                            return false;
                        }
                    } // failed to match quote, continue searching
                } 
            }
        // ESCAPE 2: pragmas, macros, includes, etc.
        } else if (c == '#') { 
            if (c == '#') {
                char prev = c;
                while (char xnewline = *YYCURSOR) {
                    if (xnewline == '\000') {
                        return false;
                    }
                    bool encounteredNewLine = false;
                    // Handle CRLF, treat \r\n as one char
                    if (xnewline == '\r') {
                        ++YYCURSOR;
                        if (*YYCURSOR == '\n') {
                            ++YYCURSOR;
                        }
                        encounteredNewLine = true;
                    } else if (*YYCURSOR == '\n') {
                        ++YYCURSOR;
                        encounteredNewLine = true;
                    } else {
                        ++YYCURSOR;
                    }
                    // escaping macros before new lines, 
                    // expands the macro to be longer, check if there is no \ character
                    if (encounteredNewLine && prev != '\\') {
                        break;
                    }
                    prev = xnewline;
                }
            }
        // ESCAPE 3: double quotes
        } else if (c == '"') {
            while (char xquote = *YYCURSOR) {
                if (xquote == '\000') {
                    return false;
                }
                // Invalid characters in quote,, c++ syntax error
                if (xquote == '\r' || xquote == '\n') {
                    ++YYCURSOR;
                    continue;
                }
                if (xquote == '\\') { // ignore escapes
                    ++YYCURSOR;
                    if (*YYCURSOR == '\000') {
                        return false;
                    }
                    ++YYCURSOR;
                    continue;
                }
                if (xquote == '"') {
                    // finished quote!
                    ++YYCURSOR;
                    break;
                } else {
                    ++YYCURSOR;
                }
            }
        // ESCAPE 4: single quotes
        } else if (c == '\'') {
            // FIXME: NUMBER ESCAPES!!!! e.g. 0xAB'CD or 1'000'000! 
            // Issue for now, it would make the complexity way worse
            while (char xquote = *YYCURSOR) {
                if (xquote == '\000') {
                    return false;
                }
                // Invalid characters in quote, c++ syntax error
                if (xquote == '\r' || xquote == '\n') {
                    ++YYCURSOR;
                    continue;
                }
                if (xquote == '\\') { // ignore escapes
                    ++YYCURSOR;
                    if (*YYCURSOR == '\000') {
                        return false;
                    }
                    ++YYCURSOR;
                    continue;
                }
                if (xquote == '\'') {
                    // finished quote!
                    ++YYCURSOR;
                    break;
                } else {
                    ++YYCURSOR;
                }
            }
        // ESCAPE 5: single line comments
        } else if (c == '/' && *YYCURSOR == '/') {
            while (char c = *++YYCURSOR) {
                if (c == '\n') {
                    ++YYCURSOR;
                    break;
                } else if (c == '\\') {
                    ++YYCURSOR;
                    if (*YYCURSOR != '\000') {
                        ++YYCURSOR;
                    }
                } 
            }
        // ESCAPE 6: multi line comments
        } else if (c == '/' && *YYCURSOR == '*') {
            while (char c = *YYCURSOR++) {
                if (c == '*' && *YYCURSOR == '/') {
                    ++YYCURSOR;
                    break;
                }
            }
        } else { 
            return c;
        }
    }
    return '\000';
}

// C++ Block `{ ... }`
// Returns true if successfully found closing block
static bool SeekTillCloseBlockBalancedWithEscape(const char*& YYCURSOR) {
    int openBraceCount = 1;
    while (char c = AdvanceCppEscaped(YYCURSOR)) {
        if (c == '{') {
            ++openBraceCount;
        } else if (c == '}') {
            --openBraceCount;
        }
        if (openBraceCount == 0) {
            return true;
        }
    }
    return false;
}


// C++ Param List `( ... )`
// Returns true if successfully found closing block
static bool SeekTillCloseParamBalancedWithEscape(const char*& YYCURSOR) {
    int openParenCount = 1;
    while (char c = AdvanceCppEscaped(YYCURSOR)) {
        if (c == '(') {
            ++openParenCount;
        } else if (c == ')') {
            --openParenCount;
        }
        if (openParenCount == 0) {
            return true;
        }
    }
    return false;
}

// C++ Expression `=> ... ;`
// Returns true if successfully found closing expression
static bool SeekTillCloseExpressionBalancedWithEscape(const char*& YYCURSOR) {
    int openBraceCount = 0;
    int openParenCount = 0;
    int openBrackCount = 0;
    while (char c = AdvanceCppEscaped(YYCURSOR)) {
        if (c == '{') {
            ++openBraceCount;
        } else if (c == '}') {
            --openBraceCount;
        } else if (c == '(') {
            ++openParenCount;
        } else if (c == ')') {
            --openParenCount;
        } else if (c == '[') {
            ++openBrackCount;
        } else if (c == ']') {
            --openBrackCount;
        }
        if (openBraceCount == 0 && c == ';') {
            return true;
        }
    }
    return false;
}

// C++ Expression `=> ... ;`
// Returns true if successfully found closing expression
static Token ScanCppType(const char*& YYCURSOR) {
    const char* start = YYCURSOR;
    int openBraceCount = 0;
    int openAngleCount = 0;
    int openBrackCount = 0;
    while (char c = AdvanceCppEscaped(YYCURSOR)) {
        if (c == '{') {
            ++openBraceCount;
        } else if (c == '}') {
            --openBraceCount;
        } else if (c == '<') {
            ++openAngleCount;
        } else if (c == '>') {
            --openAngleCount;
        } else if (c == '[') {
            ++openBrackCount;
        } else if (c == ']') {
            --openBrackCount;
        }
        if (openBraceCount == 0) {
        // C++ types end after ( or ";"
           if (c == '(' || c == ';') break;
        }
    }
    // TODO: backwards scan to check if this is valid or not!

    return {EToken_ASP_ERR, {start, static_cast<size_t>(start - YYCURSOR) } };
}

static void SkipWhitespaceAndComments(const char*& YYCURSOR) {
    while (char c = *YYCURSOR++) {
        if (c == '\n' || c == ' ' || c == '\r' || c == '\t') {
            continue;
        } else if (c == '/') {
            if (*YYCURSOR == '/') {
                while (char d = *YYCURSOR++) {
                    if (d == '\n') break;
                }
            } else if (*YYCURSOR == '*') {
                while (char d = *YYCURSOR++) {
                    if (d == '*' && *YYCURSOR == '/') {
                        ++YYCURSOR;
                        break;
                    }
                }
            } else {
                --YYCURSOR;
                break;
            }
        } else {
            --YYCURSOR;
            break;
        }
    } 
}

Token NextToken_Aspect(const char*& YYCURSOR, AspectLexerState& state) {
    const char* YYMARKER;

    SkipWhitespaceAndComments(YYCURSOR);
    const char* start = YYCURSOR;

    if (state.expectType) {
        const char* wordPtr = YYCURSOR;
        while (isalnum(*wordPtr) || *wordPtr == '_') ++wordPtr;
        std::string_view word(YYCURSOR, wordPtr - YYCURSOR);
        if (word != "inline" && word != "cache" && word != "nocache") {
            state.expectType = false;
            return ScanCppType(YYCURSOR); 
        }
    } else if (state.expectExpr) {
        state.expectExpr = false;
        const char* s = YYCURSOR;
        if (!SeekTillCloseExpressionBalancedWithEscape(YYCURSOR)) { 
            state = {}; 
            return {EToken_ASP_ERR, {s, static_cast<size_t>(YYCURSOR - s)}}; 
        }
        --YYCURSOR; // leave ; behind
        return {EToken_ASP_CPP_EXPR, {s,  static_cast<size_t>(YYCURSOR - s)}};
    }

    // Skip whitespace
    /*!re2c
        re2c:define:YYCTYPE = "char";
        re2c:yyfill:enable = 0;

        ":"  { return {EToken_ASP_COLON, {start, 1}}; }
        ";"  { return {EToken_ASP_SEMICOLON, {start, 1}}; }
        ","  { return {EToken_ASP_COMMA, {start, 1}}; }
        "::" { return {EToken_ASP_DOUBLE_COLON, {start, 2}}; }
        "=>" { state.expectExpr = true; return {EToken_ASP_ARROW, {start, 2}}; }
        "("  { 
            start = YYCURSOR;
            SeekTillCloseParamBalancedWithEscape(YYCURSOR);
            return {EToken_ASP_CPP_PARAM_LIST, {start, static_cast<size_t>(YYCURSOR - start - 1)}};
         }
        "{"  { 
            if (state.expectBrace) {
                state.expectBrace = false;
                return {EToken_ASP_L_BRACE, {start, 1}}; 
            } else {
                start = YYCURSOR;
                SeekTillCloseBlockBalancedWithEscape(YYCURSOR);
                return {EToken_ASP_CPP_BLOCK, {start, static_cast<size_t>(YYCURSOR - start - 1)}};
            }
        }
        "}"  { return {EToken_ASP_R_BRACE, {start, 1}}; }

        "aspect" { state.expectAspectID = true; return {EToken_ASP_ASPECT, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        "public" { return {EToken_ASP_PUBLIC, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        "protected" { return {EToken_ASP_PROTECTED, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        "private" { return {EToken_ASP_PRIVATE, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        "construct" { return {EToken_ASP_CONSTRUCT, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        "destruct" { return {EToken_ASP_DESTRUCT, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        "synthetic" { state.expectType = true; return {EToken_ASP_SYNTHETIC, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        "inherited" { state.expectType = true; return {EToken_ASP_INHERITED, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        "collection" { state.expectType = true; return {EToken_ASP_COLLECTION, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        "contribute" { return {EToken_ASP_CONTRIBUTE, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        "inline" { return {EToken_ASP_INLINE, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        "cache" { return {EToken_ASP_CACHE, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        "nocache" { return {EToken_ASP_NOCACHE, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        "final" { return {EToken_ASP_FINAL, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        "override" { return {EToken_ASP_OVERRIDE, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        "const" { return {EToken_ASP_CONST, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        "noexcept" { return {EToken_ASP_NOEXCEPT, {start, static_cast<size_t>(YYCURSOR - start)}}; }
        "to" { return {EToken_ASP_TO, {start, static_cast<size_t>(YYCURSOR - start)}}; }

        "%header"[ \n\r\t]*"{" {
            start = YYCURSOR;
            SeekTillCloseBlockBalancedWithEscape(YYCURSOR);
            return {EToken_ASP_B_HEADER, {start, static_cast<size_t>(YYCURSOR - start - 1)}};
        }
        "%source"[ \n\r\t]*"{" {
            start = YYCURSOR;
            SeekTillCloseBlockBalancedWithEscape(YYCURSOR);
            return {EToken_ASP_B_SOURCE, {start, static_cast<size_t>(YYCURSOR - start - 1)}};
        }

        [a-zA-Z_][a-zA-Z0-9_]* {
            if (state.expectAspectID) {
                state.expectAspectID = false;
                state.expectBrace = true;
            } 
            return {EToken_ASP_ID, {start, static_cast<size_t>(YYCURSOR - start)}};
        }

        // End of input
        "\000" { state = {}; return {EToken_ASP_EOF, {start, 0}}; }

        // Fallback catch-all error rule
        * { state = {}; return {EToken_ASP_ERR, {start, 1}}; }
    */
    assert(false && "unreachable");
}

} // zblast

#ifdef __clang__
#pragma clang diagnostic pop
#endif
