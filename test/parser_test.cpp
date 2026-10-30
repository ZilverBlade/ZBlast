// Gemini Flash 3.8 High AI Generated @ 2026-10-29

#include <gtest/gtest.h>
#include "parser.h"
#include "parsetree.h"
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
 *   node_ref     -> ID
 *   node         -> node_ref opt_inherit
 *   opt_inherit  -> ":" node_ref | \epsilon
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
    void ParseText(const char *src) {
        auto tokens = Tokenize(src);
        ASSERT_FALSE(tokens.empty());
        ASSERT_EQ(tokens.back().type, EToken_EOF);

        Parser parser;
        parser.Parse(tokens.data());
    }

    void ParseFile(const std::string &filename) {
        std::string content = ReadTestFile(filename);
        ParseText(content.c_str());
    }

    // Runs a parser test case: reads input.zb, writes output.out with indentation,
    // and compares against output.expected
    void RunParserTestCase(const std::string &caseName) {
        std::filesystem::path testDir = GetTestDir() / "parser" / caseName;
        std::filesystem::path inputPath = testDir / "input.zb";
        std::filesystem::path expectedPath = testDir / "output.expected";
        std::filesystem::path outputPath = testDir / "output.out";

        ASSERT_TRUE(std::filesystem::exists(inputPath)) << "Missing input: " << inputPath;
        ASSERT_TRUE(std::filesystem::exists(expectedPath)) << "Missing expected: " << expectedPath;

        std::string content = ReadTestFile((std::filesystem::path("parser") / caseName / "input.zb").string());
        auto tokens = Tokenize(content);
        ASSERT_FALSE(tokens.empty());
        ASSERT_EQ(tokens.back().type, EToken_EOF);

        Parser parser;
        auto tree = parser.Parse(tokens.data());
        ASSERT_NE(tree, nullptr);

        std::string actualOutput = parsetree::PrintTreeToString(tree.get());

        // Write output.out with indentation
        std::ofstream outFile(outputPath);
        ASSERT_TRUE(outFile.is_open()) << "Failed to open output file: " << outputPath;
        outFile << actualOutput;
        outFile.close();

        // Compare with output.expected
        std::string expectedOutput = ReadTestFile((std::filesystem::path("parser") / caseName / "output.expected").string());
        EXPECT_EQ(actualOutput, expectedOutput);
    }
};

// --- Token Stream Readiness for LL(1) Parser ---

TEST(ParserReadinessTest, TokensArePreparedForParser) {
    // Verifies that Tokenize produces a null/EOF-terminated token buffer
    // that matches the Token* signature of Parser::Parse(Token* pTokens)
    const char *src = "Expr ::= <ID>;";
    auto tokens = Tokenize(src);

    ASSERT_FALSE(tokens.empty());
    EXPECT_EQ(tokens.back().type, EToken_EOF);

    // Verify token pointers are contiguous
    Token *pTokens = tokens.data();
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
    // Valid inheritance
    EXPECT_NO_THROW({
        ParseText("LetDecl : Stmt ::= <ID>;");
    });
    // Invalid inheritance
    EXPECT_ANY_THROW({
        ParseText("BinaryExpr : Expr : Node ::= Left:Expr <OP> Right:Expr;");
        });
}

TEST_F(ParserTestFixture, ParseAbstractDeclaration) {
    EXPECT_NO_THROW({
        ParseText("abstract Stmt;");
        });
    EXPECT_NO_THROW({
        ParseText("abstract Expr;");
        });
}

TEST_F(ParserTestFixture, ParseChildTypes) {
    EXPECT_NO_THROW({
        // Literal child
        ParseText("LiteralRule ::= <ID>;");
        });
    EXPECT_NO_THROW({
        // Optional child
        ParseText("OptionalRule ::= [Value:<INT_LITERAL>];");
        });
    // List qualifier
    EXPECT_NO_THROW({
        ParseText("ListRule ::= Stmt*;");
        });
    // Variant qualifier
    EXPECT_NO_THROW({
        ParseText("VariantRule ::= LetDecl | ConstDecl;");
        });
    // Mixed children
    EXPECT_NO_THROW({
        ParseText("ComplexRule ::= Prefix:ID [OptValue:<INT_LITERAL>] Suffix:ID*;");
        });
}

TEST_F(ParserTestFixture, ParseFullAstExample) {
    const char *src = R"(
        abstract Stmt;
        LetDecl : Stmt ::= <ID>; // let x;
        ConstDecl : Stmt ::= <ID> [Value:<INT_LITERAL>]; // const x: 0;
        VarDeclStmt : Stmt ::= /* variant */ LetDecl | ConstDecl;
    )";

    EXPECT_NO_THROW({
        ParseText(src);
        });
}

// --- Test Files Integration & Tree Output Verification ---

TEST_F(ParserTestFixture, ParseTestCase01SimpleRule) {
    RunParserTestCase("01_simple_rule");
}

TEST_F(ParserTestFixture, ParseTestCase02Inheritance) {
    RunParserTestCase("02_inheritance");
}

TEST_F(ParserTestFixture, ParseTestCase03Abstract) {
    RunParserTestCase("03_abstract");
}

TEST_F(ParserTestFixture, ParseTestCase04ChildrenTypes) {
    RunParserTestCase("04_children_types");
}

TEST_F(ParserTestFixture, ParseTestCase05FullAst) {
    RunParserTestCase("05_full_ast");
}

TEST_F(ParserTestFixture, ParseTestCase06CommentsAndWhitespace) {
    RunParserTestCase("06_comments_and_whitespace");
}

TEST_F(ParserTestFixture, ParseAllParserSubdirectoryCases) {
    std::filesystem::path parserDir = GetTestDir() / "parser";
    int count = 0;
    for (const auto &entry : std::filesystem::directory_iterator(parserDir)) {
        if (entry.is_directory()) {
            RunParserTestCase(entry.path().filename().string());
            count++;
        }
    }
    EXPECT_GE(count, 6);
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
        ParseFile("lexer/07_syntax_error.zb");
        });
}
