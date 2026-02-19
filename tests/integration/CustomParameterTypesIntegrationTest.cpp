#include <gtest/gtest.h>
#include <cucumber-cpp/internal/utils/CucumberExpression.hpp>
#include <regex>
#include <filesystem>

using namespace cucumber::internal;

/**
 * Integration test for custom parameter types.
 * 
 * This test expects a custom_parameter_types.json file to be present in the working directory
 * with the following content:
 * 
 * [
 *     {
 *         "name": "color",
 *         "regexp": "red|blue|green|yellow"
 *     },
 *     {
 *         "name": "gender",
 *         "regexp": "male|female"
 *     }
 * ]
 * 
 * To run this test:
 * 1. Create the custom_parameter_types.json file in your working directory
 * 2. Run the test executable
 */

class CustomParameterTypesIntegrationTest : public ::testing::Test {
protected:
    void ValidateRegex(const std::string& regex) {
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

// Test that custom color type works
TEST_F(CustomParameterTypesIntegrationTest, CustomColorType) {
    // If the JSON file exists, this should work
    if (std::filesystem::exists("custom_parameter_types.json")) {
        TestExpression(
            "I have a {color} ball",
            {"I have a red ball", "I have a blue ball", "I have a green ball", "I have a yellow ball"},
            {"I have a orange ball", "I have a ball", "I have a red and blue ball"}
        );
    } else {
        GTEST_SKIP() << "custom_parameter_types.json not found - skipping integration test";
    }
}

// Test that custom gender type works
TEST_F(CustomParameterTypesIntegrationTest, CustomGenderType) {
    if (std::filesystem::exists("custom_parameter_types.json")) {
        TestExpression(
            "The person is {gender}",
            {"The person is male", "The person is female"},
            {"The person is other", "The person is"}
        );
    } else {
        GTEST_SKIP() << "custom_parameter_types.json not found - skipping integration test";
    }
}

// Test that custom types work alongside built-in types
TEST_F(CustomParameterTypesIntegrationTest, CustomAndBuiltInTypes) {
    if (std::filesystem::exists("custom_parameter_types.json")) {
        TestExpression(
            "I have {int} {color} balls",
            {"I have 5 red balls", "I have 10 blue balls", "I have 0 green balls"},
            {"I have 5 orange balls", "I have red balls", "I have 5.5 blue balls"}
        );
    } else {
        GTEST_SKIP() << "custom_parameter_types.json not found - skipping integration test";
    }
}

// Test built-in types still work even if JSON file doesn't exist
TEST_F(CustomParameterTypesIntegrationTest, BuiltInTypesAlwaysWork) {
    TestExpression(
        "I have {int} cucumbers",
        {"I have 42 cucumbers", "I have -19 cucumbers", "I have 0 cucumbers"},
        {"I have 3.5 cucumbers", "I have cucumbers", "I have abc cucumbers"}
    );
}
