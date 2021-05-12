#include "../include/text_parser.hpp"

#include <algorithm>
#include <sstream>
#include <unordered_map>
#include "../include/log.hpp"
#include "../include/xd/vendor/utf8.h"

std::string Token::to_string() const {
    if (type == Token_Type::TEXT) return value;

    std::string str = "{";
    if (type == Token_Type::CLOSING_TAG) {
        str += "/";
    }
    str += tag;
    if (type == Token_Type::OPENING_TAG && !value.empty()) {
        str += "=" + value;
    }
    str += "}";

    return str;
}

Token Token::to_opening_token(const std::string& val) const {
    if (type != Token_Type::CLOSING_TAG) return *this;

    Token opening_token = *this;
    if (!val.empty()) opening_token.value = val;
    opening_token.type = Token_Type::OPENING_TAG;
    return opening_token;
}

Token Token::to_closing_token() const {
    if (type != Token_Type::OPENING_TAG) return *this;

    Token closing_token = *this;
    closing_token.type = Token_Type::CLOSING_TAG;
    return closing_token;
}

std::vector<Token> Text_Parser::parse(const std::string& text, bool permissive) const {
    std::vector<Token> tokens;

    auto start = text.begin(), end = text.end();

    auto validate = [permissive, &text](const char* error_message) {
        if (permissive)
            return;

        std::ostringstream ss;
        ss << "Parsing failure: " << error_message << ". Text: " << text;
        throw parsing_exception(ss.str());
    };

    auto validate_condition = [permissive, validate](bool condition, const char* error_message) {
        if (condition && !permissive)
            validate(error_message);

        return condition;
    };

    std::vector<char> special{'{', '}', '/', '$', '='};
    std::unordered_map<std::string, std::vector<int>> unmatched_tokens;

    while (start != end) {
        // Handle tags
        auto next_char = start + 1 != end ? *(start + 1) : '_';
        if (*start == '{' && next_char != '{') {
            Token tag_token;
            tag_token.unmatched = false;
            tag_token.start_index = utf8::distance(text.begin(), start);
            start++;
            if (validate_condition(start == end, "open brace at the end")) break;

            if (*start == '/') {
                tag_token.type = Token_Type::CLOSING_TAG;
                start++;
                if (validate_condition(start == end, "close brace at the end"))
                    break;
            } else {
                tag_token.type = Token_Type::OPENING_TAG;
            }
            std::string tag_name;
            std::string value;
            bool has_value = false;
            bool error = false;
            while (start != end) {
                if (std::find(std::begin(special), std::end(special), *start) == std::end(special)) {
                    // Read tag name or value
                    if (tag_token.tag.empty())
                        tag_name += *start;
                    else
                        value += *start;
                } else {
                    // Handle special character in tag
                    if (*start == '=') {
                        if (tag_token.type == Token_Type::OPENING_TAG) {
                            tag_token.tag = tag_name;
                            has_value = true;
                        } else {
                            validate("equal sign in closing tag");
                            error = true;
                            break;
                        }
                    } else if (*start == '}') {
                        tag_token.end_index = utf8::distance(text.begin(), start);
                        if (tag_token.tag.empty()) {
                            if (validate_condition(tag_name.empty(), "empty tag")) {
                                error = true;
                                break;
                            }
                            tag_token.tag = tag_name;
                        } else {
                            if (validate_condition(value.empty(), "empty value")) {
                                error = true;
                                break;
                            }
                            tag_token.value = value;
                        }

                        // Closing the tag and matching it
                        if (tag_token.type == Token_Type::OPENING_TAG) {
                            unmatched_tokens[tag_token.tag].push_back(tokens.size());
                            tag_token.unmatched = true;
                        } else {
                            if (unmatched_tokens[tag_token.tag].empty()) {
                                // A closing tag with no matched open tag
                                tag_token.unmatched = true;
                            } else {
                                auto& opening_token = tokens[unmatched_tokens[tag_token.tag].back()];
                                // Found the closing tag for an unmatched open tag
                                if (opening_token.start_index < tag_token.start_index) {
                                    opening_token.unmatched = false;
                                    unmatched_tokens[tag_token.tag].pop_back();
                                }
                            }
                        }

                        start++;
                        break;
                    } else {
                        validate("unexpected character in tag");
                        error = true;
                        break;
                    }
                }
                start++;
            }

            // Reached the end of the string but tag wasn't closed
            auto unclosed_tag = tag_token.tag.empty() || (has_value && tag_token.value.empty());
            if (!error && !validate_condition(unclosed_tag, "tag was not closed "))
                tokens.push_back(tag_token);
        }

        // Not a tag token, so we read it as a text token until we find a tag
        if (start != end) {
            Token text_token;
            text_token.unmatched = false;
            text_token.start_index = utf8::distance(text.begin(), start);
            text_token.type = Token_Type::TEXT;
            std::string parsed_text;
            while (start != end)
            {
                next_char = start + 1 != end ? *(start + 1) : '_';
                auto is_text = *start != '{' && *start != '}';
                if (!is_text && *start == next_char) {
                    parsed_text += *start;
                    start++;
                } else if (is_text) {
                    parsed_text += *start;
                } else if (*start == '{') {
                    break;
                } else {
                    validate_condition(*start == '}', "unexpected closing tag");
                }

                start++;
            }

            if (!parsed_text.empty()) {
                text_token.value = parsed_text;
                text_token.end_index = parsed_text.empty() ? text_token.start_index : utf8::distance(text.begin(), start) - 1;
                tokens.push_back(text_token);
            }
        }
    }

    return tokens;
}
