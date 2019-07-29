#include "../include/text_parser.hpp"

#include <algorithm>
#include <sstream>
#include <unordered_map>
#include "../include/log.hpp"
#include "../include/xd/vendor/utf8.h"

std::vector<Token> Text_Parser::parse(const std::string& text, bool permissive) const
{
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
        if (*start == '{') {
            Token tag_token;
            tag_token.unmatched = false;
            tag_token.start_index = utf8::distance(text.begin(), start);
            start++;
            if (validate_condition(start == end, "open brace at the end"))
                break;

            if (*start == '/') {
                tag_token.type = "closing_tag";
                start++;
                if (validate_condition(start == end, "close brace at the end"))
                    break;
            } else {
                tag_token.type = "opening_tag";
            }
            std::string tag_name;
            std::string value;
            bool has_value = false;
            bool error = false;
            while (start != end)
            {
                if (std::find(std::begin(special), std::end(special), *start) == std::end(special)) {
                    if (tag_token.tag.empty())
                        tag_name += *start;
                    else
                        value += *start;
                } else {
                    if (*start == '=') {
                        if (tag_token.type == "opening_tag") {
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

                        if (tag_token.type == "opening_tag") {
                            unmatched_tokens[tag_token.tag].push_back(tokens.size());
                            tag_token.unmatched = true;
                        } else {
                            if (unmatched_tokens[tag_token.tag].empty()) {
                                tag_token.unmatched = true;
                            } else {
                                auto& opening_token = tokens[unmatched_tokens[tag_token.tag].back()];
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

            auto unclosed_tag = tag_token.tag.empty() || (has_value && tag_token.value.empty());
            if (!error && !validate_condition(unclosed_tag, "tag was not closed "))
                tokens.push_back(tag_token);
        }

        if (start != end) {
            Token text_token;
            text_token.unmatched = false;
            text_token.start_index = utf8::distance(text.begin(), start);
            text_token.type = "text";
            std::string parsed_text;
            while (start != end)
            {
                if (*start != '{' && *start != '}')
                    parsed_text += *start;
                else if (*start == '{')
                    break;

                validate_condition(*start == '}', "unexpected closing tag");
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
