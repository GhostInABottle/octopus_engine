#include <boost/test/unit_test.hpp>
#include <stdexcept>
#include "../../include/text_parser.hpp"
#include "../../include/utility/string.hpp"

namespace detail {
    static std::string invalid_parsing_cases[] = {
        "{",            // open brace at the end
        "a{",           // open brace at the end
        "a{/",          // close brace at the end
        "a}",           // unexpected closing tag
        "{b}a{/b=4}",   // equal sign in closing tag
        "{}",           // empty tag
        "{/}",          // empty tag
        "{a=}",         // empty value
        "{//}",         // unexpected character in tag
        "{/b/b}",       // unexpected character in tag
        "{a",           // tag was not closed
        "{/a",          // tag was not closed
        "aaa}"          // unexpected closing tag
    };

    static std::vector<std::string> before_split = {
        "h{b}ello",
        "{/b}{c=6}h{d}i",
        "hey{/d}",
        "b{/c}y{e}e{/e}!",
        "{icon=4/}farewell"
    };

    static std::vector<std::string> after_split = {
        "h{b}ello{/b}",
        "{b}{/b}{c=6}h{d}i{/d}{/c}",
        "{c=6}{d}hey{/d}{/c}",
        "{c=6}b{/c}y{e}e{/e}!",
        "{icon=4/}farewell"
    };

    static void validate_token(Token actual, Token expected) {
        BOOST_CHECK_EQUAL(static_cast<int>(actual.type), static_cast<int>(expected.type));
        BOOST_CHECK_EQUAL(actual.tag, expected.tag);
        BOOST_CHECK_EQUAL(actual.value, expected.value);
        BOOST_CHECK_EQUAL(actual.unmatched, expected.unmatched);
        BOOST_CHECK_EQUAL(actual.start_index, expected.start_index);
        BOOST_CHECK_EQUAL(actual.end_index, expected.end_index);
        BOOST_CHECK_EQUAL(actual.self_closing, expected.self_closing);
    }

    static void validate_tokens(const std::string& text, std::vector<Token> expectedTokens) {
        std::vector<Token> actualTokens;

        try {
            actualTokens = Text_Parser::parse(text);
        } catch (std::exception& e) {
            // Workaround because BOOST_CHECK_NO_THROW is always passing for some reason
            BOOST_FAIL(e.what());
        }

        BOOST_CHECK_EQUAL(actualTokens.size(), expectedTokens.size());

        for (std::size_t i = 0; i < actualTokens.size(); ++i) {
            validate_token(actualTokens[i], expectedTokens[i]);
        }
    }

    static Token build_token(Token_Type type, std::string tag, std::string value, bool unmatched, int start, int end, bool self_closing = false) {
        Token token;
        token.type = type;
        token.tag = tag;
        token.value = value;
        token.unmatched = unmatched;
        token.start_index = start;
        token.end_index = end;
        token.self_closing = self_closing;
        return token;
    }
}

BOOST_AUTO_TEST_SUITE(text_parser_tests)

BOOST_AUTO_TEST_CASE(text_token_to_string_returns_value) {
    auto token = detail::build_token(Token_Type::TEXT, "tag", "value", true, 0, 0);
    BOOST_CHECK_EQUAL(token.to_string(), "value");
}

BOOST_AUTO_TEST_CASE(opening_tag_token_to_string_returns_value) {
    auto token_without_value = detail::build_token(Token_Type::OPENING_TAG, "tag", "", true, 0, 0);
    auto token_with_value = detail::build_token(Token_Type::OPENING_TAG, "tag", "value", true, 0, 0);

    BOOST_CHECK_EQUAL(token_without_value.to_string(), "{tag}");
    BOOST_CHECK_EQUAL(token_with_value.to_string(), "{tag=value}");
}

BOOST_AUTO_TEST_CASE(closing_tag_token_to_string_returns_value) {
    auto token = detail::build_token(Token_Type::CLOSING_TAG, "tag", "value", true, 0, 0);
    BOOST_CHECK_EQUAL(token.to_string(), "{/tag}");
}

BOOST_AUTO_TEST_CASE(token_to_opening_token_changes_closing_token) {
    auto text_token = detail::build_token(Token_Type::TEXT, "tag", "value", true, 0, 0);
    auto opening_token = detail::build_token(Token_Type::OPENING_TAG, "tag", "value", true, 0, 0);
    auto closing_token = detail::build_token(Token_Type::CLOSING_TAG, "tag", "value", true, 0, 0);

    detail::validate_token(text_token.to_opening_token(), text_token);
    detail::validate_token(opening_token.to_opening_token(), opening_token);

    auto expected = closing_token;
    expected.type = Token_Type::OPENING_TAG;
    detail::validate_token(closing_token.to_opening_token(), expected);

    expected.value = "parameter_value";
    detail::validate_token(closing_token.to_opening_token("parameter_value"), expected);
}

BOOST_AUTO_TEST_CASE(token_to_closing_token_changes_opening_token) {
    auto text_token = detail::build_token(Token_Type::TEXT, "tag", "value", true, 0, 0);
    auto opening_token = detail::build_token(Token_Type::OPENING_TAG, "tag", "value", true, 0, 0);
    auto closing_token = detail::build_token(Token_Type::CLOSING_TAG, "tag", "value", true, 0, 0);

    detail::validate_token(text_token.to_closing_token(), text_token);
    detail::validate_token(closing_token.to_closing_token(), closing_token);

    auto expected = closing_token;
    expected.type = Token_Type::CLOSING_TAG;
    detail::validate_token(opening_token.to_closing_token(), expected);
}

BOOST_AUTO_TEST_CASE(text_parser_exceptions) {
    for (auto& invalid : detail::invalid_parsing_cases) {
        BOOST_CHECK_THROW(Text_Parser::parse(invalid), parsing_exception);
    }
}


BOOST_AUTO_TEST_CASE(text_parser_no_exceptions_if_permissive) {
    for (auto& invalid : detail::invalid_parsing_cases) {
        try {
            Text_Parser::parse(invalid, true);
        } catch (std::exception& e) {
            BOOST_FAIL(e.what());
        }
    }
}

BOOST_AUTO_TEST_CASE(text_parser_handles_empty_text) {
    std::vector<Token> tokens;
    detail::validate_tokens("", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_text) {
    std::vector<Token> tokens = {
        detail::build_token(Token_Type::TEXT, "", "hello", false, 0, 4)
    };

    detail::validate_tokens("hello", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_simple_tag) {
    std::vector<Token> tokens = {
        detail::build_token(Token_Type::OPENING_TAG, "b", "", false, 0, 2),
        detail::build_token(Token_Type::TEXT, "", "hello", false, 3, 7),
        detail::build_token(Token_Type::CLOSING_TAG, "b", "", false, 8, 11)
    };

    detail::validate_tokens("{b}hello{/b}", tokens);
}


BOOST_AUTO_TEST_CASE(text_parser_parses_unicode_text) {
    std::vector<Token> tokens = {
        detail::build_token(Token_Type::OPENING_TAG, "b", "", false, 0, 2),
        detail::build_token(Token_Type::TEXT, "", "مرحبا", false, 3, 7),
        detail::build_token(Token_Type::CLOSING_TAG, "b", "", false, 8, 11)
    };

    detail::validate_tokens("{b}مرحبا{/b}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_simple_tag_with_value) {
    std::vector<Token> tokens = {
        detail::build_token(Token_Type::OPENING_TAG, "b", "c", false, 0, 4),
        detail::build_token(Token_Type::TEXT, "", "hello", false, 5, 9),
        detail::build_token(Token_Type::CLOSING_TAG, "b", "", false, 10, 13)
    };

    detail::validate_tokens("{b=c}hello{/b}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_tag_with_forward_slash_in_value) {
    std::vector<Token> tokens = {
        detail::build_token(Token_Type::OPENING_TAG, "b", "c/,d/x,2", false, 0, 11),
        detail::build_token(Token_Type::TEXT, "", "hello", false, 12, 16),
        detail::build_token(Token_Type::CLOSING_TAG, "b", "", false, 17, 20)
    };

    detail::validate_tokens("{b=c/,d/x,2}hello{/b}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_simple_tag_with_empty_text) {
    std::vector<Token> tokens = {
        detail::build_token(Token_Type::OPENING_TAG, "b", "", false, 0, 2),
        detail::build_token(Token_Type::CLOSING_TAG, "b", "", false, 3, 6)
    };

    detail::validate_tokens("{b}{/b}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_self_closing_tags) {
    std::vector<Token> tokens = {
        detail::build_token(Token_Type::OPENING_TAG, "b", "", false, 0, 3, true),
        detail::build_token(Token_Type::CLOSING_TAG, "b", "", false, 3, 3, true),
        detail::build_token(Token_Type::TEXT, "", "x", false, 4, 4),
        detail::build_token(Token_Type::OPENING_TAG, "c", "1", false, 5, 10, true),
        detail::build_token(Token_Type::CLOSING_TAG, "c", "", false, 10, 10, true)
    };

    detail::validate_tokens("{b/}x{c=1/}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_multiple_tags) {
    std::vector<Token> tokens = {
        detail::build_token(Token_Type::OPENING_TAG, "a", "", false, 0, 2),
        detail::build_token(Token_Type::TEXT, "", "x", false, 3, 3),
        detail::build_token(Token_Type::CLOSING_TAG, "a", "", false, 4, 7),
        detail::build_token(Token_Type::OPENING_TAG, "b", "", false, 8, 10),
        detail::build_token(Token_Type::TEXT, "", "y", false, 11, 11),
        detail::build_token(Token_Type::CLOSING_TAG, "b", "", false, 12, 15)
    };

    detail::validate_tokens("{a}x{/a}{b}y{/b}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_unmatched_tags) {
    std::vector<Token> tokens = {
        detail::build_token(Token_Type::OPENING_TAG, "a", "", true, 0, 2),
        detail::build_token(Token_Type::CLOSING_TAG, "b", "", true, 3, 6),
    };

    detail::validate_tokens("{a}{/b}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_multiple_unmatched_tags) {
    std::vector<Token> tokens = {
        detail::build_token(Token_Type::OPENING_TAG, "a", "", true, 0, 2),
        detail::build_token(Token_Type::OPENING_TAG, "b", "", false, 3, 5),
        detail::build_token(Token_Type::OPENING_TAG, "a", "", false, 6, 8),
        detail::build_token(Token_Type::CLOSING_TAG, "b", "", false, 9, 12),
        detail::build_token(Token_Type::OPENING_TAG, "a", "", false, 13, 15),
        detail::build_token(Token_Type::CLOSING_TAG, "a", "", false, 16, 19),
        detail::build_token(Token_Type::CLOSING_TAG, "b", "", true, 20, 23),
        detail::build_token(Token_Type::CLOSING_TAG, "a", "", false, 24, 27)
    };

    detail::validate_tokens("{a}{b}{a}{/b}{a}{/a}{/b}{/a}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_escaped_tags) {
    std::vector<Token> tokens = {
        detail::build_token(Token_Type::TEXT, "", "{b}hello{b} world{/b}", false, 0, 26),
    };

    detail::validate_tokens("{{b}}hello{{b}} world{{/b}}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_split_to_lines_handles_empty_text) {
    auto lines = Text_Parser::split_to_lines("");
    BOOST_CHECK_EQUAL(lines.size(), 1);
    BOOST_CHECK_EQUAL(lines[0], "");
}

BOOST_AUTO_TEST_CASE(text_parser_split_to_lines_handles_single_line) {
    auto lines = Text_Parser::split_to_lines("hello");
    BOOST_CHECK_EQUAL(lines.size(), 1);
    BOOST_CHECK_EQUAL(lines[0], "hello");
}

BOOST_AUTO_TEST_CASE(text_parser_split_to_lines_handles_single_line_non_permissive) {
    auto lines = Text_Parser::split_to_lines("h{b}ello", false);
    BOOST_CHECK_EQUAL(lines.size(), 1);
    BOOST_CHECK_EQUAL(lines[0], "h{b}ello");
}

BOOST_AUTO_TEST_CASE(text_parser_split_to_lines_handles_single_line_permissive) {
    auto lines = Text_Parser::split_to_lines("h{b}ello", false);
    BOOST_CHECK_EQUAL(lines.size(), 1);
    BOOST_CHECK_EQUAL(lines[0], "h{b}ello");
}

BOOST_AUTO_TEST_CASE(text_parser_split_to_lines_throws_for_error_in_multi_line_non_permissive) {
    BOOST_CHECK_THROW(Text_Parser::split_to_lines("h{b}i\nbye{/b=1}", false), parsing_exception);
}

BOOST_AUTO_TEST_CASE(text_parser_split_to_lines_handles_multi_line_non_permissive) {
    auto lines = Text_Parser::split_to_lines(string_utilities::join(detail::before_split, "\n"), false);
    BOOST_CHECK_EQUAL_COLLECTIONS(lines.begin(), lines.end(), detail::after_split.begin(), detail::after_split.end());
}

BOOST_AUTO_TEST_CASE(text_parser_split_to_lines_handles_multi_line_permissive) {
    auto lines = Text_Parser::split_to_lines(string_utilities::join(detail::before_split, "\n"), true);
    BOOST_CHECK_EQUAL_COLLECTIONS(lines.begin(), lines.end(), detail::after_split.begin(), detail::after_split.end());
}

BOOST_AUTO_TEST_CASE(text_parser_strip_tags_returns_original_if_no_input_tags) {
    auto original = "hello{b} there{/b}!";
    auto actual = Text_Parser::strip_tags(original, {});
    BOOST_CHECK_EQUAL(actual, original);
}

BOOST_AUTO_TEST_CASE(text_parser_strip_tags_returns_original_if_tags_not_found) {
    auto original = "hello{b} there{/b}!";
    auto actual = Text_Parser::strip_tags(original, {"c"});
    BOOST_CHECK_EQUAL(actual, original);
}

BOOST_AUTO_TEST_CASE(text_parser_strip_tags_strips_tag) {
    auto original = "hello{b} there{/b}!";
    auto actual = Text_Parser::strip_tags(original, {"b", "c"});
    BOOST_CHECK_EQUAL(actual, "hello there!");
}

BOOST_AUTO_TEST_CASE(text_parser_strip_tags_handles_complex_cases) {
    auto original = "{c=5,abc}hello{b} {/w}\nt{icon=4/}here{/b}!";
    auto actual = Text_Parser::strip_tags(original, {"b", "c", "icon", "d"});
    BOOST_CHECK_EQUAL(actual, "hello {/w}\nthere!");
}

BOOST_AUTO_TEST_CASE(text_parser_strip_tags_throws_when_found_and_error_and_non_permissive) {
    BOOST_CHECK_THROW(Text_Parser::strip_tags("{a=}{b}", {"b"}, false), parsing_exception);
}

BOOST_AUTO_TEST_CASE(text_parser_strip_tags_does_not_throw_when_not_found_and_error_and_non_permissive) {
    auto actual = Text_Parser::strip_tags("{a=}{b}", {"c"}, false);
    BOOST_CHECK_EQUAL(actual, "{a=}{b}");
}

BOOST_AUTO_TEST_CASE(text_parser_strip_tags_does_not_throw_when_found_and_error_and_permissive) {
    auto actual = Text_Parser::strip_tags("{a=}{b}", {"b"}, true);
    BOOST_CHECK_EQUAL(actual, "");
}

BOOST_AUTO_TEST_CASE(text_parser_strip_tags_does_not_throw_when_not_found_and_error_and_permissive) {
    auto actual = Text_Parser::strip_tags("{a=}{b}", {"c"}, true);
    BOOST_CHECK_EQUAL(actual, "{a=}{b}");
}

BOOST_AUTO_TEST_SUITE_END()
