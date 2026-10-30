// Gemini Flash 3.8 High AI Generated @ 2026-10-29

#include <gtest/gtest.h>
#include "test_helpers.h"

using namespace zblast;
using namespace zblast::test;

// --- Single Token Tests ---

TEST(LexerTest, TokenizePunctuation) {
    const char* src = "::= : ; * | [ ] < >";
    auto tokens = Tokenize(src);

    std::vector<ETokenType> expected = {
        EToken_DOUBLE_COLON_EQ,
        EToken_COLON,
        EToken_SEMICOLON,
        EToken_STAR,
        EToken_PIPE,
        EToken_L_BRACKET,
        EToken_R_BRACKET,
        EToken_L_ANGLE,
        EToken_R_ANGLE,
        EToken_EOF
    };

    EXPECT_EQ(ExtractTypes(tokens), expected);
    EXPECT_EQ(tokens[0].text, "::=");
    EXPECT_EQ(tokens[1].text, ":");
    EXPECT_EQ(tokens[2].text, ";");
    EXPECT_EQ(tokens[3].text, "*");
    EXPECT_EQ(tokens[4].text, "|");
    EXPECT_EQ(tokens[5].text, "[");
    EXPECT_EQ(tokens[6].text, "]");
    EXPECT_EQ(tokens[7].text, "<");
    EXPECT_EQ(tokens[8].text, ">");
}

TEST(LexerTest, TokenizeAbstractKeyword) {
    const char* src = "abstract";
    auto tokens = Tokenize(src);

    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].type, EToken_ABSTRACT);
    EXPECT_EQ(tokens[1].type, EToken_EOF);
}

TEST(LexerTest, AbstractPrefixAsIdentifier) {
    const char* src = "abstract_node abstraction";
    auto tokens = Tokenize(src);

    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].type, EToken_ID);
    EXPECT_EQ(tokens[0].text, "abstract_node");
    EXPECT_EQ(tokens[1].type, EToken_ID);
    EXPECT_EQ(tokens[1].text, "abstraction");
    EXPECT_EQ(tokens[2].type, EToken_EOF);
}

TEST(LexerTest, TokenizeIdentifiers) {
    const char* src = "Stmt LetDecl _private CamelCase UPPERCASE snake_case_123";
    auto tokens = Tokenize(src);

    std::vector<ETokenType> expectedTypes = {
        EToken_ID, EToken_ID, EToken_ID, EToken_ID, EToken_ID, EToken_ID, EToken_EOF
    };
    EXPECT_EQ(ExtractTypes(tokens), expectedTypes);

    EXPECT_EQ(tokens[0].text, "Stmt");
    EXPECT_EQ(tokens[1].text, "LetDecl");
    EXPECT_EQ(tokens[2].text, "_private");
    EXPECT_EQ(tokens[3].text, "CamelCase");
    EXPECT_EQ(tokens[4].text, "UPPERCASE");
    EXPECT_EQ(tokens[5].text, "snake_case_123");
}

TEST(LexerTest, TokenizeLabels) {
    // A label is an identifier followed immediately by ':' (e.g. "Value:")
    const char* src = "Value: Name: _field:";
    auto tokens = Tokenize(src);

    // Each label emits EToken_LABEL then EToken_COLON
    std::vector<ETokenType> expected = {
        EToken_LABEL, EToken_COLON,
        EToken_LABEL, EToken_COLON,
        EToken_LABEL, EToken_COLON,
        EToken_EOF
    };
    EXPECT_EQ(ExtractTypes(tokens), expected);

    EXPECT_EQ(tokens[0].text, "Value");
    EXPECT_EQ(tokens[1].text, ":");
    EXPECT_EQ(tokens[2].text, "Name");
    EXPECT_EQ(tokens[3].text, ":");
    EXPECT_EQ(tokens[4].text, "_field");
    EXPECT_EQ(tokens[5].text, ":");
}

TEST(LexerTest, LabelVsIdWithSpaceBeforeColon) {
    // When there is space before colon, it is an ID followed by COLON, not a LABEL
    const char* src = "Value : Stmt";
    auto tokens = Tokenize(src);

    std::vector<ETokenType> expected = {
        EToken_ID, EToken_COLON, EToken_ID, EToken_EOF
    };
    EXPECT_EQ(ExtractTypes(tokens), expected);
    EXPECT_EQ(tokens[0].text, "Value");
}

// --- Comments & Whitespace Tests ---

TEST(LexerTest, LineComments) {
    const char* src = "// line comment\n"
                      "Rule1; // trailing comment\n"
                      "// another comment";
    auto tokens = Tokenize(src);

    std::vector<ETokenType> expected = {
        EToken_ID, EToken_SEMICOLON, EToken_EOF
    };
    EXPECT_EQ(ExtractTypes(tokens), expected);
    EXPECT_EQ(tokens[0].text, "Rule1");
}

TEST(LexerTest, LineCommentAtEOFWithoutNewline) {
    const char* src = "Rule1; // trailing comment without newline";
    auto tokens = Tokenize(src);

    std::vector<ETokenType> expected = {
        EToken_ID, EToken_SEMICOLON, EToken_EOF
    };
    EXPECT_EQ(ExtractTypes(tokens), expected);
}

TEST(LexerTest, BlockComments) {
    const char* src = "/* block comment */ Rule1 /* another */ ::= /* inline */ <ID>;";
    auto tokens = Tokenize(src);

    std::vector<ETokenType> expected = {
        EToken_ID, EToken_DOUBLE_COLON_EQ, EToken_L_ANGLE, EToken_ID, EToken_R_ANGLE, EToken_SEMICOLON, EToken_EOF
    };
    EXPECT_EQ(ExtractTypes(tokens), expected);
    EXPECT_EQ(tokens[0].text, "Rule1");
    EXPECT_EQ(tokens[3].text, "ID");
}

TEST(LexerTest, MultilineBlockCommentWithSpecialChars) {
    const char* src = "/*\n"
                      " * Star line\n"
                      " * / slash\n"
                      " * ** double star\n"
                      " */ Rule1;";
    auto tokens = Tokenize(src);

    std::vector<ETokenType> expected = {
        EToken_ID, EToken_SEMICOLON, EToken_EOF
    };
    EXPECT_EQ(ExtractTypes(tokens), expected);
    EXPECT_EQ(tokens[0].text, "Rule1");
}

TEST(LexerTest, WhitespaceHandling) {
    const char* src = "  \t \r\n \n\t  Rule1  \t\r\n  ::=  ;  ";
    auto tokens = Tokenize(src);

    std::vector<ETokenType> expected = {
        EToken_ID, EToken_DOUBLE_COLON_EQ, EToken_SEMICOLON, EToken_EOF
    };
    EXPECT_EQ(ExtractTypes(tokens), expected);
}

TEST(LexerTest, EmptyInput) {
    const char* src = "";
    auto tokens = Tokenize(src);

    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, EToken_EOF);
}

TEST(LexerTest, WhitespaceOnlyInput) {
    const char* src = "   \t \r\n \n";
    auto tokens = Tokenize(src);

    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, EToken_EOF);
}

// --- Error Handling Tests ---

TEST(LexerTest, LexicalErrors) {
    const char* src = "Rule @ $ % ^ ;";
    auto tokens = Tokenize(src);

    std::vector<ETokenType> expected = {
        EToken_ID,
        EToken_ERR, // @
        EToken_ERR, // $
        EToken_ERR, // %
        EToken_ERR, // ^
        EToken_SEMICOLON,
        EToken_EOF
    };
    EXPECT_EQ(ExtractTypes(tokens), expected);
    EXPECT_EQ(tokens[1].text, "@");
    EXPECT_EQ(tokens[2].text, "$");
    EXPECT_EQ(tokens[3].text, "%");
    EXPECT_EQ(tokens[4].text, "^");
}

// --- Complex Snippet & Test Files ---

TEST(LexerTest, MainSnippetTokenization) {
    const char* src = R"(
        abstract Stmt;
        LetDecl : Stmt ::= <ID>; // let x;
        ConstDecl : Stmt ::= <ID> [Value:<INT_LITERAL>]; // const x: 0;
        VarDeclStmt : Stmt ::= /* variant */ LetDecl | ConstDecl;
    )";

    auto tokens = Tokenize(src);

    std::vector<ETokenType> expected = {
        EToken_ABSTRACT, EToken_ID, EToken_SEMICOLON,
        EToken_ID, EToken_COLON, EToken_ID, EToken_DOUBLE_COLON_EQ, EToken_L_ANGLE, EToken_ID, EToken_R_ANGLE, EToken_SEMICOLON,
        EToken_ID, EToken_COLON, EToken_ID, EToken_DOUBLE_COLON_EQ, EToken_L_ANGLE, EToken_ID, EToken_R_ANGLE,
        EToken_L_BRACKET, EToken_LABEL, EToken_COLON, EToken_L_ANGLE, EToken_ID, EToken_R_ANGLE, EToken_R_BRACKET, EToken_SEMICOLON,
        EToken_ID, EToken_COLON, EToken_ID, EToken_DOUBLE_COLON_EQ, EToken_ID, EToken_PIPE, EToken_ID, EToken_SEMICOLON,
        EToken_EOF
    };

    EXPECT_EQ(ExtractTypes(tokens), expected);
}

TEST(LexerTest, TestFile01SimpleRule) {
    std::string content = ReadTestFile("lexer/01_simple_rule.zast");
    auto tokens = Tokenize(content);

    std::vector<ETokenType> expected = {
        EToken_ID, EToken_DOUBLE_COLON_EQ, EToken_L_ANGLE, EToken_ID, EToken_R_ANGLE, EToken_SEMICOLON, EToken_EOF
    };
    EXPECT_EQ(ExtractTypes(tokens), expected);
    EXPECT_EQ(tokens[0].text, "Expr");
    EXPECT_EQ(tokens[3].text, "ID");
}

TEST(LexerTest, TestFile02Inheritance) {
    std::string content = ReadTestFile("lexer/02_inheritance.zast");
    auto tokens = Tokenize(content);

    EXPECT_EQ(tokens[0].type, EToken_ID);
    EXPECT_EQ(tokens[0].text, "LetDecl");
    EXPECT_EQ(tokens[1].type, EToken_COLON);
    EXPECT_EQ(tokens[2].type, EToken_ID);
    EXPECT_EQ(tokens[2].text, "Stmt");
    EXPECT_EQ(tokens[3].type, EToken_DOUBLE_COLON_EQ);

    EXPECT_EQ(tokens.back().type, EToken_EOF);
}

TEST(LexerTest, TestFile03Abstract) {
    std::string content = ReadTestFile("lexer/03_abstract.zast");
    auto tokens = Tokenize(content);

    std::vector<ETokenType> expected = {
        EToken_ABSTRACT, EToken_ID, EToken_SEMICOLON,
        EToken_ABSTRACT, EToken_ID, EToken_SEMICOLON,
        EToken_EOF
    };
    EXPECT_EQ(ExtractTypes(tokens), expected);
    EXPECT_EQ(tokens[1].text, "Stmt");
    EXPECT_EQ(tokens[4].text, "Expr");
}

TEST(LexerTest, TestFile04ChildrenTypes) {
    std::string content = ReadTestFile("lexer/04_children_types.zast");
    auto tokens = Tokenize(content);

    // Make sure no ERR tokens exist in valid file
    for (const auto& tok : tokens) {
        EXPECT_NE(tok.type, EToken_ERR) << "Unexpected ERR token with text: " << tok.text;
    }
    EXPECT_EQ(tokens.back().type, EToken_EOF);
}

TEST(LexerTest, TestFile05FullAst) {
    std::string content = ReadTestFile("lexer/05_full_ast.zast");
    auto tokens = Tokenize(content);

    for (const auto& tok : tokens) {
        EXPECT_NE(tok.type, EToken_ERR) << "Unexpected ERR token: " << tok.text << " @ " << tok.lineStart << ":" << tok.colStart;
    }
    EXPECT_EQ(tokens.back().type, EToken_EOF);
}

TEST(LexerTest, TestFile06CommentsAndWhitespace) {
    std::string content = ReadTestFile("lexer/06_comments_and_whitespace.zast");
    auto tokens = Tokenize(content);

    for (const auto& tok : tokens) {
        EXPECT_NE(tok.type, EToken_ERR) << "Unexpected ERR token: " << tok.text << " @ " << tok.lineStart << ":" << tok.colStart;
    }
    EXPECT_EQ(tokens.back().type, EToken_EOF);
}

TEST(LexerTest, TestFile07SyntaxErrorIsLexicallyValid) {
    // 07_syntax_error.zast has syntax errors for the parser, but all characters are valid tokens
    std::string content = ReadTestFile("lexer/07_syntax_error.zast");
    auto tokens = Tokenize(content);

    // All tokens should be valid (not ERR)
    for (const auto& tok : tokens) {
        EXPECT_NE(tok.type, EToken_ERR);
    }
    EXPECT_EQ(tokens.back().type, EToken_EOF);
}

TEST(LexerTest, TestFile08LexicalErrorContainsErrTokens) {
    std::string content = ReadTestFile("lexer/08_lexical_error.zast");
    auto tokens = Tokenize(content);

    bool foundErr = false;
    for (const auto& tok : tokens) {
        if (tok.type == EToken_ERR) {
            foundErr = true;
            break;
        }
    }
    EXPECT_TRUE(foundErr) << "Expected to find EToken_ERR in 08_lexical_error.zast";
}

TEST(LexerTest, TestFile09Empty) {
    std::string content = ReadTestFile("lexer/09_empty.zast");
    auto tokens = Tokenize(content);

    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, EToken_EOF);
}
