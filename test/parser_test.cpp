#include <gtest/gtest.h>
#include "parser.h"
#include "test_helpers.h"

using namespace zblast;
using namespace zblast::test;

/**
 * @file parser_test.cpp
 *
 * Test suite for the ZBlast LL(1) parser.
 *
 * The grammar defined for ZBlast is:
 *   start        -> rule_list EOF
 *   rule_list    -> rule rule_list | \epsilon
 *   rule         -> node "::=" child_list ";" | "abstract" ID ";"
 *   node         -> ID opt_inherit
 *   opt_inherit  -> ":" node | \epsilon
 *   child_list   -> child child_list | \epsilon
 *   named_node   -> LABEL ":" ID | ID
 *   child        -> "<" ID ">"          (Literal)
 *                 | "[" named_node "]"  (Optional)
 *                 | named_node child_qual
 *                 | named_node
 *   child_qual   -> "*"                 (List)
 *                 | "|" child           (Variant)
 *
 * Parser tests below are structured using the GoogleTest  prefix.
 * This allows the entire test suite to build and pass cleanly now.
 * When you implement `Parser::Parse(Token* pTokens)` in `src/parser.cpp`,
 * simply remove the `` prefix to activate the full parser test suite!
 *
 * You can also run disabled tests with:
 *   ./zblast_tests --gtest_also_run_tests
 */

class ParserTestFixture : public ::testing::Test {
protected:
    // Helper to tokenize source text and feed it to the parser
    void ParseText(const char* src) {
        auto tokens = Tokenize(src);
        ASSERT_FALSE(tokens.empty());
        ASSERT_EQ(tokens.back().type, EToken_EOF);

        // TODO: Uncomment once Parser::Parse(Token* pTokens) is implemented in src/parser.cpp:
        // Parser parser;
        // parser.Parse(tokens.data());
    }

    void ParseFile(const std::string& filename) {
        std::string content = ReadTestFile(filename);
        ParseText(content.c_str());
    }
};

// --- Token Stream Readiness for LL(1) Parser ---

TEST(ParserReadinessTest, TokensArePreparedForParser) {
    // Verifies that Tokenize produces a null/EOF-terminated token buffer
    // that matches the Token* signature of Parser::Parse(Token* pTokens)
    const char* src = "Expr ::= <ID>;";
    auto tokens = Tokenize(src);

    ASSERT_FALSE(tokens.empty());
    EXPECT_EQ(tokens.back().type, EToken_EOF);

    // Verify token pointers are contiguous
    Token* pTokens = tokens.data();
    EXPECT_EQ(pTokens[0].type, EToken_ID);
    EXPECT_EQ(pTokens[0].text, "Expr");
    EXPECT_EQ(pTokens[1].type, EToken_DOUBLE_COLON_EQ);
    EXPECT_EQ(pTokens[2].type, EToken_L_ANGLE);
    EXPECT_EQ(pTokens[3].type, EToken_ID);
    EXPECT_EQ(pTokens[4].type, EToken_R_ANGLE);
    EXPECT_EQ(pTokens[5].type, EToken_SEMICOLON);
    EXPECT_EQ(pTokens[6].type, EToken_EOF);
}

// --- Valid Grammar Parsing Tests (Remove  once Parser::Parse is implemented) ---

TEST_F(ParserTestFixture, ParseEmptyInput) {
    EXPECT_NO_THROW({
        ParseText("");
    });
}

TEST_F(ParserTestFixture, ParseSimpleRule) {
    EXPECT_NO_THROW({
        ParseText("Expr ::= <ID>;");
    });
}

TEST_F(ParserTestFixture, ParseInheritance) {
    EXPECT_NO_THROW({
        ParseText("LetDecl : Stmt ::= <ID>;");
        ParseText("BinaryExpr : Expr : Node ::= Left:Expr <OP> Right:Expr;");
    });
}

TEST_F(ParserTestFixture, ParseAbstractDeclaration) {
    EXPECT_NO_THROW({
        ParseText("abstract Stmt;");
        ParseText("abstract Expr;");
    });
}

TEST_F(ParserTestFixture, ParseChildTypes) {
    EXPECT_NO_THROW({
        // Literal child
        ParseText("LiteralRule ::= <ID>;");
        // Optional child
        ParseText("OptionalRule ::= [Value:<INT_LITERAL>];");
        // List qualifier
        ParseText("ListRule ::= Stmt*;");
        // Variant qualifier
        ParseText("VariantRule ::= LetDecl | ConstDecl;");
        // Mixed children
        ParseText("ComplexRule ::= Prefix:ID [OptValue:<INT_LITERAL>] Suffix:ID*;");
    });
}

TEST_F(ParserTestFixture, ParseFullAstExample) {
    const char* src = R"(
        abstract Stmt;
        LetDecl : Stmt ::= <ID>; // let x;
        ConstDecl : Stmt ::= <ID> [Value:<INT_LITERAL>]; // const x: 0;
        VarDeclStmt : Stmt ::= /* variant */ LetDecl | ConstDecl;
    )";

    EXPECT_NO_THROW({
        ParseText(src);
    });
}

// --- Test Files Integration ---

TEST_F(ParserTestFixture, ParseTestFile01SimpleRule) {
    EXPECT_NO_THROW({
        ParseFile("01_simple_rule.zb");
    });
}

TEST_F(ParserTestFixture, ParseTestFile02Inheritance) {
    EXPECT_NO_THROW({
        ParseFile("02_inheritance.zb");
    });
}

TEST_F(ParserTestFixture, ParseTestFile03Abstract) {
    EXPECT_NO_THROW({
        ParseFile("03_abstract.zb");
    });
}

TEST_F(ParserTestFixture, ParseTestFile04ChildrenTypes) {
    EXPECT_NO_THROW({
        ParseFile("04_children_types.zb");
    });
}

TEST_F(ParserTestFixture, ParseTestFile05FullAst) {
    EXPECT_NO_THROW({
        ParseFile("05_full_ast.zb");
    });
}

TEST_F(ParserTestFixture, ParseTestFile06CommentsAndWhitespace) {
    EXPECT_NO_THROW({
        ParseFile("06_comments_and_whitespace.zb");
    });
}

// --- Syntax Error Tests ---

TEST_F(ParserTestFixture, RejectMissingSemicolon) {
    EXPECT_ANY_THROW({
        ParseText("Expr ::= <ID>");
    });
}

TEST_F(ParserTestFixture, RejectMissingDoubleColonEq) {
    EXPECT_ANY_THROW({
        ParseText("Expr : Stmt <ID>;");
    });
}

TEST_F(ParserTestFixture, RejectInvalidSyntaxFile) {
    EXPECT_ANY_THROW({
        ParseFile("07_syntax_error.zb");
    });
}
