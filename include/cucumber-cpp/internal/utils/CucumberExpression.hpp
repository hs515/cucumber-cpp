#ifndef CUKE_CUCUMBER_EX_HPP_
#define CUKE_CUCUMBER_EX_HPP_

#include <map>
#include <stdexcept>
#include <string>

namespace cucumber {
namespace internal {

/**
 * Custom exception classes for Cucumber Expression parsing and validation errors
 */

/**
 * Base exception for all Cucumber Expression errors
 */
class CucumberExpressionpressionException : public std::invalid_argument {
public:
    explicit CucumberExpressionpressionException(const std::string& message)
        : std::invalid_argument(message) {}
};

/**
 * Thrown when a parameter type is unknown or invalid
 */
class UnknownParameterTypeException : public CucumberExpressionpressionException {
public:
    UnknownParameterTypeException(const std::string& paramType)
        : CucumberExpressionpressionException("Unknown parameter type: {" + paramType + "}"),
          paramType_(paramType) {}
    
    const std::string& getParameterType() const { return paramType_; }

private:
    std::string paramType_;
};

/**
 * Thrown when braces/parameter types are not properly closed
 */
class UnclosedParameterException : public CucumberExpressionpressionException {
public:
    explicit UnclosedParameterException(const std::string& message = "Unclosed parameter type: missing '}'")
        : CucumberExpressionpressionException(message) {}
};

/**
 * Thrown when parentheses/optional text are not properly closed
 */
class UnclosedOptionalException : public CucumberExpressionpressionException {
public:
    explicit UnclosedOptionalException(const std::string& message = "Unclosed optional text: missing ')'")
        : CucumberExpressionpressionException(message) {}
};

/**
 * Thrown when closing braces are found without matching opening braces
 */
class UnmatchedClosingBraceException : public CucumberExpressionpressionException {
public:
    UnmatchedClosingBraceException()
        : CucumberExpressionpressionException("Unexpected closing brace '}' without matching opening brace") {}
};

/**
 * Thrown when closing parentheses are found without matching opening parentheses
 */
class UnmatchedClosingParenthesisException : public CucumberExpressionpressionException {
public:
    UnmatchedClosingParenthesisException()
        : CucumberExpressionpressionException("Unexpected closing parenthesis ')' without matching opening parenthesis") {}
};

/**
 * Thrown when an optional text is empty (contains no characters)
 */
class EmptyOptionalException : public CucumberExpressionpressionException {
public:
    explicit EmptyOptionalException(int column = -1)
        : CucumberExpressionpressionException(
            column >= 0 ? "An optional must contain some text (at column " + std::to_string(column) + ")"
                       : "An optional must contain some text") {}
};

/**
 * Thrown when parameter types are placed inside optional text
 */
class OptionalParameterTypeException : public CucumberExpressionpressionException {
public:
    explicit OptionalParameterTypeException(int column = -1)
        : CucumberExpressionpressionException(
            column >= 0 ? "An optional may not contain a parameter type (at column " + std::to_string(column) + ")"
                       : "An optional may not contain a parameter type") {}
};

/**
 * Thrown when alternation is found inside optional text
 */
class AlternationInOptionalException : public CucumberExpressionpressionException {
public:
    AlternationInOptionalException()
        : CucumberExpressionpressionException("Alternation is not allowed inside optional text") {}
};

/**
 * Thrown when alternation has an empty alternative
 */
class EmptyAlternativeException : public CucumberExpressionpressionException {
public:
    explicit EmptyAlternativeException(int column = -1)
        : CucumberExpressionpressionException(
            column >= 0 ? "Alternation may not have empty alternatives (at column " + std::to_string(column) + ")"
                       : "Alternation may not have empty alternatives") {}
};

/**
 * Thrown when nesting optional text is attempted
 */
class NestedOptionalException : public CucumberExpressionpressionException {
public:
    NestedOptionalException()
        : CucumberExpressionpressionException("Nested optional text is not allowed") {}
};

/**
 * Thrown when the expression is empty
 */
class EmptyExpressionException : public CucumberExpressionpressionException {
public:
    EmptyExpressionException()
        : CucumberExpressionpressionException("Cucumber expression cannot be empty") {}
};

class cukex {
public:
    /**
     * Converts a Cucumber Expression string to a regular expression string.
     * 
     * References:
     * - https://github.com/cucumber/cucumber-expressions#readme
     * - https://cucumber.github.io/try-cucumber-expressions/?advanced=1
     * 
     * Supported Cucumber Expression Features:
     * 
     * 1. Literal text:
     *    "I have {int} cucumbers" matches "I have 42 cucumbers"
     * 
     * 2. Parameter types (built-in):
     *    - {int}          : Matches integers (e.g., -19, 42)
     *    - {float}        : Matches floats (e.g., 3.6, .8, -9.2)
     *    - {word}         : Matches words without whitespace (e.g., banana)
     *    - {string}       : Matches quoted strings (e.g., "banana split" or 'banana split')
     *    - {bigdecimal}   : Matches floats as BigDecimal
     *    - {double}       : Matches floats as 64-bit float
     *    - {biginteger}   : Matches integers as BigInteger
     *    - {byte}         : Matches integers as 8-bit signed integer
     *    - {short}        : Matches integers as 16-bit signed integer
     *    - {long}         : Matches integers as 64-bit signed integer
     *    - {}             : Anonymous parameter - matches anything (.*)
     * 
     * 3. Optional text (with parentheses):
     *    "I have {int} cucumber(s)" matches both "I have 1 cucumber" and "I have 42 cucumbers"
     * 
     * 4. Alternative text (with forward slash, no whitespace between parts):
     *    "I have {int} cucumber(s) in my belly/stomach" matches text with either "belly" or "stomach"
     * 
     * 5. Escaping:
     *    - \( escapes literal opening parenthesis
     *    - \{ escapes literal opening brace
     *    - \/ escapes literal forward slash
     * 
     * @param expression A valid Cucumber Expression string
     * @return A regular expression string (with ^ and $ anchors for full match)
     * @throws std::invalid_argument if the expression is invalid or not a valid Cucumber expression
     * @throws std::runtime_error if the expression is malformed
     * 
     * Examples:
     *   cukex::transform("I have {int} cucumbers")
     *   -> "^I have (-?\\d+) cucumbers$"
     *   
     *   cukex::transform("there is/are {int} flight(s)")
     *   -> "^there (?:is|are) (-?\\d+) flight(?:s)?$"
     *   
     *   cukex::transform("I have \\{literal\\} braces")
     *   -> "^I have \\{literal\\} braces$"
     */
    static std::string transform(const std::string& expression);
};

} // namespace internal
} // namespace cucumber

#endif // CUKE_CUCUMBER_EX_HPP_
