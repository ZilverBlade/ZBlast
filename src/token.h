#pragma once

#include <string>
#include <string_view>

namespace zblast {

#define TOKENS(X) \
    X(ABSTRACT, "abstract") \
    X(ID, "<identifier>") X(LABEL, "<label>") \
    X(COLON, ":") \
    X(DOUBLE_COLON_EQ, "::=") \
    X(SEMICOLON, ";") \
    X(STAR, "*") \
    X(PIPE, "|") \
    X(L_BRACKET, "[") X(R_BRACKET, "]") \
    X(L_ANGLE, "<") X(R_ANGLE, ">") \
    X(EOF, "<end of file>") \
    X(ERR, "<error>") \
\
    X(ASP_ASPECT, "aspect") \
    X(ASP_PUBLIC, "public") \
    X(ASP_PROTECTED, "protected") \
    X(ASP_PRIVATE, "private") \
    X(ASP_CONSTRUCT, "construct") \
    X(ASP_DESTRUCT, "destruct") \
    X(ASP_SYNTHETIC, "synthetic") \
    X(ASP_INHERITED, "inherited") \
    X(ASP_COLLECTION, "collection") \
    X(ASP_CONTRIBUTE, "contribute") \
    X(ASP_INLINE, "inline") \
    X(ASP_CACHE, "cache") \
    X(ASP_NOCACHE, "nocache") \
    X(ASP_FINAL, "final") \
    X(ASP_OVERRIDE, "override") \
    X(ASP_CONST, "const") \
    X(ASP_NOEXCEPT, "noexcept") \
    X(ASP_TO, "to") \
    X(ASP_ID, "<identifier>") \
    X(ASP_CPP_BLOCK, "<C++ block>") /*class::func() <qual> { ... }*/\
    X(ASP_CPP_EXPR, "<C++ expression>") /*class::func() <qual> => ... ;*/ \
    X(ASP_CPP_TYPE, "<C++ type>") /*asp ... class::func()*/ \
    X(ASP_CPP_PARAM_LIST, "<C++ params>") /*class::func(...)*/ \
    X(ASP_B_HEADER, "%header") /*%header { ... }*/\
    X(ASP_B_SOURCE, "%source") /*%source { ... }*/\
    X(ASP_COLON, ":") \
    X(ASP_SEMICOLON, ";") \
    X(ASP_COMMA, ",") \
    X(ASP_DOUBLE_COLON, "::") \
    X(ASP_ARROW, "=>") \
    X(ASP_L_BRACE, "{") \
    X(ASP_R_BRACE, "}") \
    X(ASP_EOF, "<end of file>") \
    X(ASP_ERR, "<error>") 

#define EXPAND_TOKEN_ENUM(x, ...) EToken_##x,
    enum ETokenType {
        TOKENS(EXPAND_TOKEN_ENUM)
    };
#undef EXPAND_TOKEN_ENUM
#define EXPAND_TOKEN_TOSTRING(x, ...) case EToken_##x: return #x;
#define EXPAND_TOKEN_TOSTRING2(x, y, ...) case EToken_##x: return y;
    static const char* ETokenType_ToString(ETokenType token) {
        switch (token) {
            TOKENS(EXPAND_TOKEN_TOSTRING)
        default:
            return "???";
        }
    }
    static const char* ETokenType_ToString_Literal(ETokenType token) {
        switch (token) {
            TOKENS(EXPAND_TOKEN_TOSTRING2)
        default:
            return "???";
        }
    }
#undef EXPAND_TOKEN_TOSTRING
#undef EXPAND_TOKEN_TOSTRING2
#undef TOKENS

    struct Token {
        ETokenType type;
        std::string_view text;
        int lineStart;
        int colStart;
        int lineEnd;
        int colEnd;
    };

    static std::string Token_ToString(const Token& token) {
        if (token.type == EToken_ERR || token.type == EToken_ID || token.type == EToken_LABEL) {
            return std::string(token.text);
        } else {
            return ETokenType_ToString_Literal(token.type);
        }
    }
} // zblast
