#include "../include/text_parser.hpp"

#include <algorithm>
#include <sstream>
#include <unordered_map>
#include "../include/log.hpp"
#include "../include/utility/string.hpp"
#include "../include/xd/vendor/utf8.h"

std::string Token::to_string() const {
    if (type == Token_Type::CLOSING_TAG && self_closing) return "";
    if (type == Token_Type::TEXT) return value;

    std::string str = "{";
    if (type == Token_Type::CLOSING_TAG) {
        str += "/";
    }
    str += tag;
    if (type == Token_Type::OPENING_TAG && !value.empty()) {
        str += "=" + value;
    }
    if (type == Token_Type::OPENING_TAG && self_closing) {
        str += "/";
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

std::vector<Token> Text_Parser::parse(const std::string& text, bool permissive) {
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
            Token tag_token, close_token;
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

            auto match_closing_tag = [&unmatched_tokens, &tokens](Token& tag_token) {
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
            };

            auto close_tag = [&]() {
                tag_token.end_index = utf8::distance(text.begin(), start);
                if (tag_token.tag.empty()) {
                    if (validate_condition(tag_name.empty(), "empty tag")) {
                        error = true;
                        return false;
                    }
                    tag_token.tag = tag_name;
                } else {
                    if (validate_condition(value.empty(), "empty value")) {
                        error = true;
                        return false;
                    }
                    tag_token.value = value;
                }

                // Closing the tag and matching it
                if (tag_token.type == Token_Type::OPENING_TAG) {
                    unmatched_tokens[tag_token.tag].push_back(tokens.size());
                    tag_token.unmatched = true;
                } else {
                    match_closing_tag(tag_token);
                }
                return true;
            };

            // Read tag name and value
            while (start != end) {
                auto is_special = std::find(std::begin(special), std::end(special), *start) != std::end(special);
                auto next = start + 1;
                auto has_tag_name = !tag_token.tag.empty();
                if (is_special && *start == '/' && next != end && *next != '}' && has_tag_name) {
                    // Allow / in arguments
                    is_special = false;
                }

                if (!is_special) {
                    // Read tag name or value
                    if (!has_tag_name)
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
                        if (!close_tag()) break;
                        start++;
                        break;
                    } else if (*start == '/') {
                        // Handle self-closing tokens, e.g. {a/}
                        start++;
                        if (validate_condition(start == end, "close brace at the end of tag")) break;
                        if (validate_condition(*start != '}', "unexpected / in tag")) break;

                        tag_token.self_closing = true;
                        if (!close_tag()) break;

                        close_token.type = Token_Type::CLOSING_TAG;
                        close_token.tag = tag_token.tag;
                        close_token.start_index = tag_token.end_index;
                        close_token.end_index = tag_token.end_index;
                        close_token.self_closing = true;
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

            // Add tokens unless they were not properly closed
            auto unclosed_tag = tag_token.tag.empty() || (has_value && tag_token.value.empty());
            if (!error && !validate_condition(unclosed_tag, "tag was not closed ")) {
                tokens.push_back(tag_token);
                if (!close_token.tag.empty()) {
                    match_closing_tag(close_token);
                    tokens.push_back(close_token);
                }
            }
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

std::vector<std::string> Text_Parser::split_to_lines(const std::string& text, bool permissive) {
    // Split tags across multiple lines
    // e.g. "{a=b}x\ny{/a}" => "{a=b}x{/a}", "{a=b}y{/a}"
    auto text_lines = string_utilities::split(text, "\n", false);
    if (text_lines.empty()) {
        text_lines.push_back("");
    }
    if (text_lines.size() == 1 && !permissive) {
        return text_lines;
    }

    std::string open_tags;
    for (auto& line : text_lines) {
        line = open_tags + line;
        open_tags = "";

        auto line_tokens = parse(line, permissive);
        for (auto i = line_tokens.rbegin(); i != line_tokens.rend(); i++) {
            auto& token = *i;
            if (token.unmatched && token.type == Token_Type::OPENING_TAG) {
                // Close open tag and remember it for following lines
                line += token.to_closing_token().to_string();
                open_tags = token.to_string() + open_tags;
            }
        }
    }
    return text_lines;
}

std::string Text_Parser::strip_tags(const std::string& original, const std::unordered_set<std::string>& tags_to_strip, bool permissive) {
    auto found = false;
    for (auto& tag : tags_to_strip) {
        if (original.find(tag + "}") != std::string::npos) {
            found = true;
            break;
        }
    }
    if (!found) return original;

    auto tokens = parse(original, permissive);
    std::string result;
    for (auto& token : tokens) {
        if (!token.tag.empty() && tags_to_strip.find(token.tag) != tags_to_strip.end()) continue;
        result += token.to_string();
    }
    return result;
}
