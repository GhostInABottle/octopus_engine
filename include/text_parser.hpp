#ifndef HPP_TEXT_PARSER
#define HPP_TEXT_PARSER

#include <string>
#include <vector>
#include <stdexcept>

struct Token {
    std::string type;
    std::string tag;
    std::string value;
    bool unmatched;
    int start_index;
    int end_index;
};

// Thrown when text being parsed is invalid
class parsing_exception : public std::runtime_error {
public:
    parsing_exception(const std::string& error) : std::runtime_error(error.c_str()) {}
};

// Analyzes text and returns list of tokens
class Text_Parser {
public:
    std::vector<Token> Parse(const std::string& text, bool permissive = false) const;
};

#endif
