#include <gtest/gtest.h>
#include <cucumber-cpp/internal/utils/CucumberExpression.hpp>
#include <regex>

using namespace cucumber::internal;

class CucumberExpressionpressionTest : public ::testing::Test {
protected:
    void ValidateRegex(const std::string& regex) {
        // Ensure the result is a valid regex by compiling it
        try {
            std::regex testRegex(regex);
        } catch (const std::regex_error& e) {
            FAIL() << "Generated regex is invalid: " << regex << "\nError: " << e.what();
        }
    }
    
    void TestExpression(const std::string& expression, 
                       const std::vector<std::string>& matchingTexts,
                       const std::vector<std::string>& nonMatchingTexts) {
        std::string regex = cukex::transform(expression);
        ValidateRegex(regex);
        
        std::regex pattern(regex);
        
        for (const auto& text : matchingTexts) {
            EXPECT_TRUE(std::regex_match(text, pattern)) 
                << "Expression '" << expression << "' should match '" << text << "'\n"
                << "Generated regex: " << regex;
        }
        
        for (const auto& text : nonMatchingTexts) {
            EXPECT_FALSE(std::regex_match(text, pattern)) 
                << "Expression '" << expression << "' should NOT match '" << text << "'\n"
                << "Generated regex: " << regex;
        }
    }
};

// Test basic literal text
TEST_F(CucumberExpressionpressionTest, SimpleLiteralText) {
    TestExpression(
        "I have cucumbers",
        {"I have cucumbers"},
        {"I have apple", "I have", "have cucumbers"}
    );
}

// Test int parameter type
TEST_F(CucumberExpressionpressionTest, IntParameterType) {
    TestExpression(
        "I have {int} cucumbers",
        {"I have 42 cucumbers", "I have -19 cucumbers", "I have 0 cucumbers"},
        {"I have 3.5 cucumbers", "I have cucumbers", "I have abc cucumbers"}
    );
}

// Test float parameter type
TEST_F(CucumberExpressionpressionTest, FloatParameterType) {
    TestExpression(
        "I have {float} cucumbers",
        {"I have 3.6 cucumbers", "I have .8 cucumbers", "I have -9.2 cucumbers"},
        {"I have cucumbers", "I have abc cucumbers"}
    );
}

// Test word parameter type
TEST_F(CucumberExpressionpressionTest, WordParameterType) {
    TestExpression(
        "I like {word}",
        {"I like banana", "I like apple"},
        {"I like banana split", "I like", "I like  "}
    );
}

// Test string parameter type
TEST_F(CucumberExpressionpressionTest, StringParameterType) {
    TestExpression(
        "I say {string}",
        {"I say \"hello\"", "I say 'hello'", "I say \"banana split\"", "I say \"\""},
        {"I say hello", "I say", "I say 'hello\""}
    );
}

// Test anonymous parameter type
TEST_F(CucumberExpressionpressionTest, AnonymousParameterType) {
    TestExpression(
        "I have {}",
        {"I have anything", "I have 123", "I have !@#"},
        {"I have"}
    );
}

// Test optional text with single letter
TEST_F(CucumberExpressionpressionTest, OptionalTextSingle) {
    TestExpression(
        "I have {int} cucumber(s)",
        {"I have 1 cucumber", "I have 42 cucumbers"},
        {"I have cucumber", "I have 42 cucumber s"}
    );
}

// Test optional text with multiple characters
TEST_F(CucumberExpressionpressionTest, OptionalTextMultiple) {
    TestExpression(
        "I have {int} carrot(s) in my bag",
        {"I have 1 carrot in my bag", "I have 5 carrots in my bag"},
        {"I have in my bag", "I have 1 carrot s in my bag"}
    );
}

// Test complex expression with multiple parameters
TEST_F(CucumberExpressionpressionTest, MultipleParameters) {
    TestExpression(
        "I have {int} {word} in my {word}",
        {"I have 42 apples in my basket", "I have 1 orange in my backpack"},
        {"I have apples in my basket", "I have 42 in my basket"}
    );
}

// Test expression with literal braces (escaped)
TEST_F(CucumberExpressionpressionTest, EscapedBraces) {
    TestExpression(
        "I have \\{int\\} literal braces",
        {"I have {int} literal braces"},
        {"I have 42 literal braces", "I have literal braces"}
    );
}

// Test expression with literal parentheses (escaped)
TEST_F(CucumberExpressionpressionTest, EscapedParentheses) {
    TestExpression(
        "test \\(something\\)",
        {"test (something)"},
        {"test something", "test somethings"}
    );
}

// Test alternative text
TEST_F(CucumberExpressionpressionTest, AlternativeText) {
    TestExpression(
        "I have {int} cucumber(s) in my belly/stomach",
        {"I have 1 cucumber in my belly", "I have 1 cucumber in my stomach",
         "I have 42 cucumbers in my belly", "I have 42 cucumbers in my stomach"},
        {"I have cucumber in my belly", "I have 1 cucumber in my chest"}
    );
}

// Test invalid expressions
TEST_F(CucumberExpressionpressionTest, EmptyExpression) {
    EXPECT_THROW(
        cukex::transform(""),
        std::invalid_argument
    );
}

TEST_F(CucumberExpressionpressionTest, UnclosedBrace) {
    EXPECT_THROW(
        cukex::transform("I have {int cucumbers"),
        std::invalid_argument
    );
}

TEST_F(CucumberExpressionpressionTest, UnmatchedClosingBrace) {
    EXPECT_THROW(
        cukex::transform("I have } cucumbers"),
        std::invalid_argument
    );
}

TEST_F(CucumberExpressionpressionTest, UnclosedParenthesis) {
    EXPECT_THROW(
        cukex::transform("I have (int cucumbers"),
        std::invalid_argument
    );
}

TEST_F(CucumberExpressionpressionTest, UnmatchedClosingParenthesis) {
    EXPECT_THROW(
        cukex::transform("I have ) cucumber"),
        std::invalid_argument
    );
}

TEST_F(CucumberExpressionpressionTest, UnknownParameterType) {
    EXPECT_THROW(
        cukex::transform("I have {unknown} cucumbers"),
        std::invalid_argument
    );
}

TEST_F(CucumberExpressionpressionTest, NestedParameterTypes) {
    EXPECT_THROW(
        cukex::transform("I have {{int}} cucumbers"),
        std::invalid_argument
    );
}

// Test all built-in parameter types
TEST_F(CucumberExpressionpressionTest, AllBuiltInTypes) {
    std::string regex = cukex::transform(
        "{int} {float} {word} {string} {bigdecimal} {double} {biginteger} {byte} {short} {long} {}"
    );
    ValidateRegex(regex);
}

// Test long expression
TEST_F(CucumberExpressionpressionTest, ComplexRealWorldExpression) {
    std::string expr = "I have {int} {word}(s) in my {word} and I want {string}";
    std::string regex = cukex::transform(expr);
    ValidateRegex(regex);
    
    std::regex pattern(regex);
    EXPECT_TRUE(std::regex_match("I have 5 apples in my basket and I want \"more\"", pattern));
    EXPECT_FALSE(std::regex_match("I have apple in my basket and I want more", pattern));
}

// Test escaped forward slash in alternative context
TEST_F(CucumberExpressionpressionTest, EscapedForwardSlash) {
    TestExpression(
        "test \\/ path",
        {"test / path"},
        {"test path", "test  path"}
    );
}

// Test mixed optional and parameter types
TEST_F(CucumberExpressionpressionTest, MixedOptionalAndParameters) {
    TestExpression(
        "I have {int} item(s) and {int} backup(s)",
        {"I have 1 item and 2 backups", "I have 5 items and 1 backup"},
        {"I have item and backup", "I have 1 and 2"}
    );
}

// Test very long literal text
TEST_F(CucumberExpressionpressionTest, LongLiteralText) {
    std::string longText = "The quick brown fox jumps over the lazy dog";
    TestExpression(
        longText,
        {longText},
        {"The quick brown fox jumps over the lazy cat", "The quick brown"}
    );
}

// Test numeric corner cases
TEST_F(CucumberExpressionpressionTest, NumericCornerCases) {
    // Test negative numbers, zero, large numbers
    TestExpression(
        "value is {int}",
        {"value is -999999", "value is 0", "value is 999999", "value is -1"},
        {"value is 12.34", "value is", "value is abc"}
    );
}

// Test special characters in literal text
TEST_F(CucumberExpressionpressionTest, SpecialCharactersInLiterals) {
    TestExpression(
        "test string: [value]",
        {"test string: [value]"},
        {"test string: value", "test string: [value"}
    );
}
