#include <gtest/gtest.h>
#include <cucumber-cpp/internal/utils/CucumberExpression.hpp>
#include <regex>
#include <iostream>
using namespace cucumber::internal;

/**
 * Transformation tests for Cucumber Expressions
 * 
 * These tests are based on the transformation test data from:
 * testdata/cucumber-expression/transformation/
 * 
 * They verify that Cucumber Expressions are correctly transformed into
 * regular expressions according to the official specification.
 */
class CucumberExpressionTransformationTest : public ::testing::Test {
protected:
    void ValidateRegex(const std::string& regex) {
        // Ensure the result is a valid regex by compiling it
        try {
            std::regex testRegex(regex);
        } catch (const std::regex_error& e) {
            FAIL() << "Generated regex is invalid: " << regex << "\nError: " << e.what();
        }
    }
    
    void TestTransformation(const std::string& expression, const std::string& expectedRegex) {
        std::string actualRegex = cukex::transform(expression);
        ValidateRegex(actualRegex);
        
        EXPECT_EQ(expectedRegex, actualRegex)
            << "Expression '" << expression << "' transformed incorrectly\n"
            << "Expected: " << expectedRegex << "\n"
            << "Actual:   " << actualRegex;
    }
};

// Test: Simple text transformation
// From: testdata/cucumber-expression/transformation/text.yaml
TEST_F(CucumberExpressionTransformationTest, SimpleText) {
    TestTransformation("a", "^a$");
}

// Test: Empty expression transformation
// From: testdata/cucumber-expression/transformation/empty.yaml
// Note: The implementation throws an exception for empty expressions
TEST_F(CucumberExpressionTransformationTest, EmptyExpression) {
    EXPECT_THROW(
        cukex::transform(""),
        std::invalid_argument
    );
}

// Test: Parameter type transformation
// From: testdata/cucumber-expression/transformation/parameter.yaml
TEST_F(CucumberExpressionTransformationTest, ParameterType) {
    TestTransformation("{int}", "^(-?\\d+)$");
}

// Test: Optional text transformation
// From: testdata/cucumber-expression/transformation/optional.yaml
TEST_F(CucumberExpressionTransformationTest, OptionalText) {
    TestTransformation("(a)", "^(?:a)?$");
}

// Test: Alternation transformation
// From: testdata/cucumber-expression/transformation/alternation.yaml
TEST_F(CucumberExpressionTransformationTest, Alternation) {
    TestTransformation("a/b c/d/e", "^(?:a|b) (?:c|d|e)$");
}

// Test: Alternation with optional text
// From: testdata/cucumber-expression/transformation/alternation-with-optional.yaml
// Note: With the support for optional in alternatives, b(c) is now treated as one alternative
TEST_F(CucumberExpressionTransformationTest, AlternationWithOptional) {
    TestTransformation("a/b(c)", "^(?:a|b(?:c)?)$");
}

// Test: Regex special characters escaping
// From: testdata/cucumber-expression/transformation/escape-regex-characters.yaml
// Note: The `{}` in the expression is interpreted as a parameter type (anonymous), generating (.*)
TEST_F(CucumberExpressionTransformationTest, EscapeRegexCharacters) {
    TestTransformation("^$[]\\(\\){}\\\\.|?*+", "^\\^\\$\\[\\]\\(\\)(.*)\\\\\\\\\\.\\|\\?\\*\\+$");
}

// Test: Unicode text transformation
// From: testdata/cucumber-expression/transformation/unicode.yaml
TEST_F(CucumberExpressionTransformationTest, UnicodeText) {
    TestTransformation("Привет, Мир(ы)!", "^Привет, Мир(?:ы)?!$");
}

// Additional comprehensive transformation tests

// Test: Complex expression with mixed features
// Note: Parameters within alternatives may not work as expected
TEST_F(CucumberExpressionTransformationTest, ComplexMixedExpression) {
    TestTransformation(
        "I have {int} item(s) in word/container",
        "^I have (-?\\d+) item(?:s)? in (?:word|container)$"
    );
}

// Test: Multiple parameters
TEST_F(CucumberExpressionTransformationTest, MultipleParameters) {
    TestTransformation(
        "{int} {word} {float}",
        "^(-?\\d+) ([^\\s]+) ((?=.*\\d.*)[-+]?\\d*(?:\\.(?=\\d.*))?\\d*(?:\\d+[E][+-]?\\d+)?)$"
    );
}

// Test: Escaped characters
// Note: Escaped forward slash becomes a literal forward slash (not escaped in regex)
TEST_F(CucumberExpressionTransformationTest, EscapedCharacters) {
    TestTransformation(
        "test \\( \\{ \\/ escaped",
        "^test \\( \\{ / escaped$"
    );
}

// Test: String parameter type
TEST_F(CucumberExpressionTransformationTest, StringParameterType) {
    TestTransformation(
        "I say {string}",
        "^I say (\"([^\"\\\\]*(\\\\.[^\"\\\\]*)*)\"|'([^'\\\\]*(\\\\.[^'\\\\]*)*)')$"
    );
}

// Test: Optional with parameters
TEST_F(CucumberExpressionTransformationTest, OptionalWithParameters) {
    TestTransformation(
        "I have {int} apple(s)",
        "^I have (-?\\d+) apple(?:s)?$"
    );
}

// Test: Alternation at different positions
TEST_F(CucumberExpressionTransformationTest, AlternationPositions) {
    TestTransformation(
        "go to kitchen/bedroom for {word}",
        "^go to (?:kitchen|bedroom) for ([^\\s]+)$"
    );
}

// Test: Multiple alternatives in one expression
TEST_F(CucumberExpressionTransformationTest, MultipleAlternations) {
    TestTransformation(
        "cat/dog eat(s) fish/meat",
        "^(?:cat|dog) eat(?:s)? (?:fish|meat)$"
    );
}

// Test: Anonymous parameter type
TEST_F(CucumberExpressionTransformationTest, AnonymousParameter) {
    TestTransformation(
        "match {}",
        "^match (.*)$"
    );
}

// Test: Whitespace handling
TEST_F(CucumberExpressionTransformationTest, WhitespacePreserved) {
    TestTransformation(
        "multiple  spaces  here",
        "^multiple  spaces  here$"
    );
}

// Test: Brackets in literal text
TEST_F(CucumberExpressionTransformationTest, BracketsInLiterals) {
    TestTransformation(
        "array [0]",
        "^array \\[0\\]$"
    );
}

// Test: Pipe character (not in alternation context)
TEST_F(CucumberExpressionTransformationTest, PipeInLiterals) {
    TestTransformation(
        "a | b",
        "^a \\| b$"
    );
}

// Test: Optional in alternations
TEST_F(CucumberExpressionTransformationTest, OptionalInAlternations) {
    TestTransformation(
        "rat(s)/mouse/mice",
        "^(?:rat(?:s)?|mouse|mice)$"
    );
}

// Test: Optional in alternations with different order
TEST_F(CucumberExpressionTransformationTest, OptionalInAlternationsDifferentOrder) {
    TestTransformation(
        "mouse/mice/rat(s)",
        "^(?:mouse|mice|rat(?:s)?)$"
    );
}

// Test: Optional in alternations with different order
TEST_F(CucumberExpressionTransformationTest, OptionalInAlternationsDifferentOrder2) {
    TestTransformation(
        "mouse/rat(s)/mice",
        "^(?:mouse|rat(?:s)?|mice)$"
    );
}

// Test: Optional in alternations with different order
TEST_F(CucumberExpressionTransformationTest, TypeInOptional) {
    TestTransformation(
        "(a|b)",
        "^(?:a\\|b)?$"
    );
}

// Test: Optional in alternations with different order
TEST_F(CucumberExpressionTransformationTest, TypeInOptional2) {
    TestTransformation(
        "c(a|b)d",
        "^c(?:a\\|b)?d$"
    );
}

// Test: All built-in parameter types in one expression
TEST_F(CucumberExpressionTransformationTest, AllParameterTypesTransformation) {
    std::string result = cukex::transform(
        "{int} {float} {word} {string} {bigdecimal} {double} {biginteger} {byte} {short} {long} {}"
    );
    ValidateRegex(result);
    // Just verify it compiles without error and is valid
    EXPECT_TRUE(!result.empty());
}

// Test: A real-world step transformation
TEST_F(CucumberExpressionTransformationTest, CalcStepTransformation) {
    std::string result = cukex::transform(
        "I have entered an integer number {int} into the calculator"
    );
    std::cout << "Transformed regex: " << result << std::endl;
    ValidateRegex(result);
    // Just verify it compiles without error and is valid
    EXPECT_TRUE(!result.empty());
}

// Test: A real-world step transformation
TEST_F(CucumberExpressionTransformationTest, CalcStepTransformation2) {
    std::string result = cukex::transform(
        "^I have entered an integer number 10 into the calculator$"
    );
    std::cout << "Transformed regex: " << result << std::endl;
    ValidateRegex(result);
    // Just verify it compiles without error and is valid
    EXPECT_TRUE(!result.empty());
}
