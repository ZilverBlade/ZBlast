// Gemini Flash 3.8 High AI Generated @ 2026-10-30

#include <gtest/gtest.h>
#include "parser.h"
#include "visitors/error_visitor.h"
#include "test_helpers.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace zblast;
using namespace zblast::test;

/**
 * @file ast_test.cpp
 *
 * Test suite for AST files and ErrorVisitor.
 * Reads the test directories in test_files/ast/, runs ErrorVisitor on the AST,
 * writes the resulting output to output.out in each directory,
 * and compares it with output.expected.
 */

class AstTestFixture : public ::testing::Test {
protected:
    static std::string NormalizeLineEndings(const std::string &str) {
        std::string result;
        result.reserve(str.size());
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == '\r') {
                if (i + 1 < str.size() && str[i + 1] == '\n') {
                    continue;
                }
                result.push_back('\n');
            } else {
                result.push_back(str[i]);
            }
        }
        return result;
    }

    // Runs an AST / ErrorVisitor test case:
    // 1. Reads input.zast
    // 2. Tokenizes and parses into parsetree::Start
    // 3. Runs ErrorVisitor on the AST
    // 4. Formats errors using Error::PrettyPrint()
    // 5. Writes output to output.out in the test case directory
    // 6. Compares actual output with output.expected
    void RunAstTestCase(const std::string &caseName) {
        std::filesystem::path testDir = GetTestDir() / "ast" / caseName;
        std::filesystem::path inputPath = testDir / "input.zast";
        std::filesystem::path expectedPath = testDir / "output.expected";
        std::filesystem::path outputPath = testDir / "output.out";

        ASSERT_TRUE(std::filesystem::exists(inputPath)) << "Missing input: " << inputPath;
        ASSERT_TRUE(std::filesystem::exists(expectedPath)) << "Missing expected: " << expectedPath;

        std::string content = ReadTestFile((std::filesystem::path("ast") / caseName / "input.zast").string());
        auto tokens = Tokenize(content);
        ASSERT_FALSE(tokens.empty());
        ASSERT_EQ(tokens.back().type, EToken_EOF);

        Parser parser;
        auto tree = parser.Parse(tokens.data());
        ASSERT_NE(tree, nullptr);

        visitors::ErrorVisitor errorVisitor;
        errorVisitor.Visit(tree.get(), nullptr);
        const auto &errors = errorVisitor.GetErrors();

        std::string actualOutput;
        for (size_t i = 0; i < errors.size(); ++i) {
            if (i > 0) {
                actualOutput += "\n";
            }
            actualOutput += errors[i].PrettyPrint();
        }

        // Write output.out in the test case directory
        std::ofstream outFile(outputPath, std::ios::out | std::ios::binary | std::ios::trunc);
        ASSERT_TRUE(outFile.is_open()) << "Failed to open output file: " << outputPath;
        outFile << actualOutput;
        outFile.close();

        // Read and compare with output.expected
        std::string expectedOutput = ReadTestFile((std::filesystem::path("ast") / caseName / "output.expected").string());
        EXPECT_EQ(NormalizeLineEndings(actualOutput), NormalizeLineEndings(expectedOutput))
            << "Mismatch in AST test case: " << caseName;
    }
};

// --- Individual AST Directory Test Cases ---

TEST_F(AstTestFixture, TestCase01ValidBasic) {
    RunAstTestCase("01_valid_basic");
}

TEST_F(AstTestFixture, TestCase02ValidComplex) {
    RunAstTestCase("02_valid_complex");
}

TEST_F(AstTestFixture, TestCase03ValidUnused) {
    RunAstTestCase("03_valid_unused");
}

TEST_F(AstTestFixture, TestCase04InvalidDuplicateRule) {
    RunAstTestCase("04_invalid_duplicate_rule");
}

TEST_F(AstTestFixture, TestCase05InvalidDuplicateMember) {
    RunAstTestCase("05_invalid_duplicate_member");
}

TEST_F(AstTestFixture, TestCase06InvalidUndefinedRef) {
    RunAstTestCase("06_invalid_undefined_ref");
}


TEST_F(AstTestFixture, TestCase07InvalidUndefinedInherit) {
    RunAstTestCase("07_invalid_undefined_inheit");
}

// --- Dynamic Directory Reading Test ---

TEST_F(AstTestFixture, RunAllAstSubdirectoryCases) {
    std::filesystem::path astDir = GetTestDir() / "ast";
    ASSERT_TRUE(std::filesystem::exists(astDir)) << "AST directory not found: " << astDir;

    std::vector<std::string> cases;
    for (const auto &entry : std::filesystem::directory_iterator(astDir)) {
        if (entry.is_directory()) {
            cases.push_back(entry.path().filename().string());
        }
    }
    std::sort(cases.begin(), cases.end());

    EXPECT_GE(cases.size(), 6u);
    for (const auto &caseName : cases) {
        RunAstTestCase(caseName);
    }
}
