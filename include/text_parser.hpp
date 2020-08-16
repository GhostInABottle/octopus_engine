#ifndef HPP_TEXT_PARSER
#define HPP_TEXT_PARSER

#include <string>
#include <vector>
#include <stdexcept>

enum class Token_Type {
    TEXT,
    OPENING_TAG,
    CLOSING_TAG
};

struct Token {
    Token_Type type;
    std::string tag;
    std::string value;
    bool unmatched = false;
    int start_index = 0;
    int end_index = 0;
    std::string to_string() const;
    Token to_opening_token(const std::string& val = "") const;
    Token to_closing_token() const;
};

// Thrown when text being parsed is invalid
class parsing_exception : public std::runtime_error {
public:
    parsing_exception(const std::string& error) : std::runtime_error(error.c_str()) {}
};

// Analyzes text and returns list of tokens
class Text_Parser {
public:
    std::vector<Token> parse(const std::string& text, bool permissive = false) const;
};

#endif
