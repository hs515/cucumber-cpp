#include <cucumber-cpp/internal/utils/CucumberExpression.hpp>

#include <cctype>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace cucumber::internal {

namespace {

/**
 * cukex::transform - Convert Cucumber Expression to Regex
 * 
 * References:
 * - https://github.com/cucumber/cucumber-expressions#readme
 * - https://cucumber.github.io/try-cucumber-expressions/?advanced=1
 * 
 * Cucumber Expressions support:
 * - Parameter types: {int}, {float}, {word}, {string}, {bigdecimal}, {double},
 *   {biginteger}, {byte}, {short}, {long}, {} (anonymous)
 * - Optional text: (text)
 * - Alternative text: text1/text2 (no whitespace between parts)
 * - Escaping: \( \{ \/ to match literal characters
 * 
 * Built-in parameter type regexes:
 * - {int}: -?\d+
 * - {float}: (?=.*\d.*)[-+]?\d*(?:\.(?=\d.*))?\d*(?:\d+[E][+-]?\d+)?
 * - {word}: [^\s]+
 * - {string}: "([^"\\]*(\\.[^"\\]*)*)"|'([^'\\]*(\\.[^'\\]*)*)'
 * - {bigdecimal}: same as {float}
 * - {double}: same as {float}
 * - {biginteger}: same as {int}
 * - {byte}: same as {int}
 * - {short}: same as {int}
 * - {long}: same as {int}
 * - {} (anonymous): .*
 */
inline const std::map<std::string, std::string, std::less<>>& getParameterTypes() {
    // Built-in parameter types mapping
    static const std::map<std::string, std::string, std::less<>> PARAMETER_TYPES = {
        {"int",         R"(-?\d+)"},
        {"float",       R"((?=.*\d.*)[-+]?\d*(?:\.(?=\d.*))?\d*(?:\d+[E][+-]?\d+)?)"},
        {"word",        R"([^\s]+)"},
        {"string",      R"xyz("([^"\\]*(\\.[^"\\]*)*)"|'([^'\\]*(\\.[^'\\]*)*)')xyz"},  // because the raw string contains )", so use xyz as delimiter
        {"bigdecimal",  R"((?=.*\d.*)[-+]?\d*(?:\.(?=\d.*))?\d*(?:\d+[E][+-]?\d+)?)"},
        {"double",      R"((?=.*\d.*)[-+]?\d*(?:\.(?=\d.*))?\d*(?:\d+[E][+-]?\d+)?)"},
        {"biginteger",  R"(-?\d+)"},
        {"byte",        R"(-?\d+)"},
        {"short",       R"(-?\d+)"},
        {"long",        R"(-?\d+)"},
        {"", ".*"}  // anonymous parameter type
    };
    return PARAMETER_TYPES;
}

void validateRegex(const std::string& regex) {
    try {
        std::regex testRegex(regex);
    } catch (const std::regex_error& e) {
        throw CucumberExpressionpressionException("Failed to create valid regex: " + std::string(e.what()));
    }
}
} // anonymous namespace

class CucumberExpressionpressionParser {
private:
    std::string expression;
    size_t pos = 0;
    
public:
    explicit CucumberExpressionpressionParser(const std::string& expr) :
        expression(expr) {}

    std::string parse() {
        std::string result;
        
        while (pos < expression.length()) {
            char current = expression[pos];
            
            // Handle escaping - \( \) \{ \} \/
            if (current == '\\' && pos + 1 < expression.length()) {
                char next = expression[pos + 1];
                if (next == '(' || next == ')' || next == '{' || next == '}' || next == '/') {
                    parseEscaping(result);
                    continue;
                }
            }
            
            // Handle parameter types {type}
            if (current == '{') {
                parseOpenBrace(result);
                continue;
            }
            
            // Handle optional text (text)
            if (current == '(') {
                parseOpenParenthesis(result);
                continue;
            }
            
            // Handle alternatives - if we see a /, look back to find word boundaries
            if (current == '/') {
                parseAlternatives(result);
                continue;
            }
            
            // Regular character - escape regex special characters if needed
            if (std::string(".^$|()[]{}*+?\\").find(current) != std::string::npos) {
                result += "\\";
                result += current;
            } else {
                result += current;
            }
            pos++;
        }
        
        return result;
    }
    
private:
    // Collect words separated by / starting from the position before first /
    std::vector<std::string> collectAlternativesFromHere(size_t startPos) {
        std::vector<std::string> alternatives;
        size_t i = startPos;
        std::string current;
        
        // First, backtrack to get the word before the first /
        while (expression[i] != ' ' && expression[i] != '{') {
            char c = expression[i];
            // If we hit a closing paren, include the entire parenthesized expression
            if (c == ')') {
                // Find the matching opening paren
                int depth = 1;
                current = c + current;
                if (i == 0) break;
                i--;
                while (i >= 0 && depth > 0) {
                    char pc = expression[i];
                    current = pc + current;
                    if (pc == ')') depth++;
                    if (pc == '(') depth--;
                    if (i == 0) break;
                    i--;
                }
                continue;
            }
            // Stop backtracking at opening paren (without including it)
            if (c == '(') {
                break;
            }
            current = c + current;
            if (i == 0) break;
            i--;
        }
        
        // Now collect forward from startPos to get all alternatives
        // Add the first alternative (from backtracking)
        if (!current.empty()) {
            alternatives.push_back(current);
            current.clear();
        }
        
        std::string allWords;
        size_t fwdPos = startPos + 1;
        
        while (fwdPos < expression.length()) {
            char c = expression[fwdPos];
            
            // Stop at boundaries
            if (c == ' ' || c == '{') {
                break;
            }
            
            // If we hit '(' right after a slash (current is empty), it's outside the alternation
            if (c == '(' && current.empty()) {
                break;
            }
            
            // If we hit '(', include everything up to the matching ')' as part of the alternative
            if (c == '(') {
                size_t parenEnd = expression.find(')', fwdPos);
                if (parenEnd != std::string::npos) {
                    // Include parentheses and contents
                    while (fwdPos <= parenEnd) {
                        current += expression[fwdPos];
                        fwdPos++;
                    }
                    continue;
                }
                current += c;
                fwdPos++;
                continue;
            }
            
            // Handle escaped slash
            if (c == '\\' && fwdPos + 1 < expression.length() && expression[fwdPos + 1] == '/') {
                allWords += '/';
                fwdPos += 2;
                continue;
            }
            
            // Unescaped slash is separator
            if (c == '/') {
                // Save current word
                if (!current.empty()) {
                    alternatives.push_back(current);
                    current.clear();
                }
                fwdPos++;
                continue;
            }
            
            current += c;
            fwdPos++;
        }
        
        if (!current.empty()) {
            alternatives.push_back(current);
        }
        
        pos = fwdPos;
        return alternatives;
    }
    
    std::string escapeRegexChars(const std::string& text) const {
        std::string result;
        for (char c : text) {
            if (std::string(".^$|()[]{}*+?\\").find(c) != std::string::npos) {
                result += "\\";
            }
            result += c;
        }
        return result;
    }

    // Parse a single alternative text to handle optional content
    std::string parseAlternativeText(const std::string& text) const {
        std::string result;
        size_t i = 0;
        
        while (i < text.length()) {
            char c = text[i];
            
            // Handle optional text (text)
            if (c == '(') {
                size_t closePos = text.find(')', i);
                if (closePos != std::string::npos) {
                    std::string optionalContent = text.substr(i + 1, closePos - i - 1);
                    // Validate that optional content is not empty
                    if (optionalContent.empty()) {
                        throw CucumberExpressionpressionException("Empty optional text is not allowed");
                    }
                    std::string escapedContent = escapeRegexChars(optionalContent);
                    result += "(?:" + escapedContent + ")?";
                    i = closePos + 1;
                    continue;
                }
            }
            
            // Escape regex special characters
            if (std::string(".^$|()[]{}*+?\\").find(c) != std::string::npos) {
                result += "\\";
            }
            result += c;
            i++;
        }
        
        return result;
    }

    void parseEscaping(std::string& result) {
        char next = expression[pos + 1];
        // Output the escaped character for regex - manually escape special chars
        if (next == '{' || next == '}' || next == '(' || next == ')') {
            result += "\\";
        }
        result += next;
        pos += 2;
    }

    void parseOpenBrace(std::string& result) {
        size_t closePos = expression.find('}', pos);
        if (closePos == std::string::npos) {
            throw UnclosedParameterException("Unclosed parameter type: missing '}' at position " + 
                                   std::to_string(pos));
        }

        std::string paramType = expression.substr(pos + 1, closePos - pos - 1);

        // Validate and convert parameter type
        if (getParameterTypes().find(paramType) == getParameterTypes().end()) {
            throw UnknownParameterTypeException(paramType);
        }

        result += "(" + getParameterTypes().at(paramType) + ")";
        pos = closePos + 1;
    }

    void parseOpenParenthesis(std::string& result) {
        size_t closePos = expression.find(')', pos);
        if (closePos == std::string::npos) {
            throw UnclosedOptionalException("Unclosed optional text: missing ')' at position " + 
                                   std::to_string(pos));
        }

        std::string optionalContent = expression.substr(pos + 1, closePos - pos - 1);

        // Validate that optional content is not empty
        if (optionalContent.empty()) {
            throw CucumberExpressionpressionException("Empty optional text is not allowed at position " + 
                                   std::to_string(pos));
        }

        // Escape optional content
        std::string escapedContent = escapeRegexChars(optionalContent);
        result += "(?:" + escapedContent + ")?";
        pos = closePos + 1;
    }

    void parseAlternatives(std::string& result) {
        // Check if this is a word-level alternative (not a literal escape)
        // Backtrack to find the word before the slash
        size_t wordEnd = pos;
        while (wordEnd > 0 && result.back() != ' ' && result.back() != ')' && 
                result.back() != ')') {
            wordEnd--;
        }
        
        // For now, handle slash by looking ahead to collect all alternatives
        std::vector<std::string> alternatives = collectAlternativesFromHere(pos - 1);
        
        if (alternatives.size() > 1) {
            // Remove the last word from result
            if (size_t lastSpace = result.rfind(' '); lastSpace != std::string::npos) {
                result = result.substr(0, lastSpace + 1);
            } else {
                result.clear();
            }
            
            // Add alternatives group
            result += "(?:";
            for (size_t i = 0; i < alternatives.size(); ++i) {
                if (i > 0) result += "|";
                result += parseAlternativeText(alternatives[i]);
            }
            result += ")";
        }
    }
};

std::string cukex::transform(const std::string& expression) {
    if (expression.empty()) {
        throw EmptyExpressionException();
    }
    
    // Validate that the expression is well-formed
    int braceDepth = 0;
    int parenDepth = 0;
    
    for (size_t i = 0; i < expression.length(); ++i) {
        char c = expression[i];
        
        // Check for unescaped opening/closing braces and parentheses
        if (i > 0 && expression[i - 1] == '\\') {
            continue; // Skip escaped characters
        }
        
        if (c == '{') {
            braceDepth++;
            if (braceDepth > 1) {
                throw CucumberExpressionpressionException("Nested parameter types are not allowed");
            }
        } else if (c == '}') {
            braceDepth--;
            if (braceDepth < 0) {
                throw UnmatchedClosingBraceException();
            }
        } else if (c == '(') {
            parenDepth++;
        } else if (c == ')') {
            parenDepth--;
            if (parenDepth < 0) {
                throw UnmatchedClosingParenthesisException();
            }
        }
    }
    
    if (braceDepth != 0) {
        throw UnclosedParameterException("Unclosed parameter type: unmatched braces");
    }
    if (parenDepth != 0) {
        throw UnclosedOptionalException("Unclosed optional text: unmatched parentheses");
    }
    
    // Preprocess to convert alternatives
    std::string processedExpression = expression;
    
    // Parse and convert
    try {
        CucumberExpressionpressionParser parser(processedExpression);
        std::string regex = parser.parse();
        
        // Add anchors to ensure full match
        regex = "^" + regex + "$";

        // Validate that the result is a valid regex by trying to compile it. If invalid, throw exception.
        validateRegex(regex);
        return regex;
    } catch (const std::exception& e) {
        throw CucumberExpressionpressionException(std::string("Invalid Cucumber expression: ") + e.what());
    }
}

} // namespace cucumber::internal
