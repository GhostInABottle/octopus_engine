#include <boost/test/unit_test.hpp>
#include "../../include/text_parser.hpp"

namespace detail {
    std::string invalid_cases[] = {
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

    void validate_token(Token actual, Token expected) {
        BOOST_CHECK_EQUAL(static_cast<int>(actual.type), static_cast<int>(expected.type));
        BOOST_CHECK_EQUAL(actual.tag, expected.tag);
        BOOST_CHECK_EQUAL(actual.value, expected.value);
        BOOST_CHECK_EQUAL(actual.unmatched, expected.unmatched);
        BOOST_CHECK_EQUAL(actual.start_index, expected.start_index);
        BOOST_CHECK_EQUAL(actual.end_index, expected.end_index);
        BOOST_CHECK_EQUAL(actual.self_closing, expected.self_closing);
    }

    void validate_tokens(const Text_Parser& parser, const std::string& text, std::vector<Token> expectedTokens) {
        std::vector<Token> actualTokens;

        BOOST_CHECK_NO_THROW(actualTokens = parser.parse(text));
        BOOST_CHECK_EQUAL(actualTokens.size(), expectedTokens.size());

        for (std::size_t i = 0; i < actualTokens.size(); ++i) {
            validate_token(actualTokens[i], expectedTokens[i]);
        }
    }

    Token build_token(Token_Type type, std::string tag, std::string value, bool unmatched, int start, int end, bool self_closing = false) {
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
    Text_Parser parser;

    for (auto& invalid : detail::invalid_cases) {
        BOOST_CHECK_THROW(parser.parse(invalid), parsing_exception);
    }
}


BOOST_AUTO_TEST_CASE(text_parser_no_exceptions_if_permissive) {
    Text_Parser parser;

    for (auto& invalid : detail::invalid_cases) {
        BOOST_CHECK_NO_THROW(parser.parse(invalid, true));
    }
}

BOOST_AUTO_TEST_CASE(text_parser_handles_empty_text) {
    Text_Parser parser;

    std::vector<Token> tokens;

    detail::validate_tokens(parser, "", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_text) {
    Text_Parser parser;

    std::vector<Token> tokens = {
        detail::build_token(Token_Type::TEXT, "", "hello", false, 0, 4)
    };

    detail::validate_tokens(parser, "hello", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_simple_tag) {
    Text_Parser parser;

    std::vector<Token> tokens = {
        detail::build_token(Token_Type::OPENING_TAG, "b", "", false, 0, 2),
        detail::build_token(Token_Type::TEXT, "", "hello", false, 3, 7),
        detail::build_token(Token_Type::CLOSING_TAG, "b", "", false, 8, 11)
    };

    detail::validate_tokens(parser, "{b}hello{/b}", tokens);
}


BOOST_AUTO_TEST_CASE(text_parser_parses_unicode_text) {
    Text_Parser parser;

    std::vector<Token> tokens = {
        detail::build_token(Token_Type::OPENING_TAG, "b", "", false, 0, 2),
        detail::build_token(Token_Type::TEXT, "", "مرحبا", false, 3, 7),
        detail::build_token(Token_Type::CLOSING_TAG, "b", "", false, 8, 11)
    };

    detail::validate_tokens(parser, "{b}مرحبا{/b}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_simple_tag_with_value) {
    Text_Parser parser;

    std::vector<Token> tokens = {
        detail::build_token(Token_Type::OPENING_TAG, "b", "c", false, 0, 4),
        detail::build_token(Token_Type::TEXT, "", "hello", false, 5, 9),
        detail::build_token(Token_Type::CLOSING_TAG, "b", "", false, 10, 13)
    };

    detail::validate_tokens(parser, "{b=c}hello{/b}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_simple_tag_with_empty_text) {
    Text_Parser parser;

    std::vector<Token> tokens = {
        detail::build_token(Token_Type::OPENING_TAG, "b", "", false, 0, 2),
        detail::build_token(Token_Type::CLOSING_TAG, "b", "", false, 3, 6)
    };

    detail::validate_tokens(parser, "{b}{/b}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_self_closing_tags) {
    Text_Parser parser;

    std::vector<Token> tokens = {
        detail::build_token(Token_Type::OPENING_TAG, "b", "", false, 0, 3, true),
        detail::build_token(Token_Type::CLOSING_TAG, "b", "", false, 3, 3, true),
        detail::build_token(Token_Type::TEXT, "", "x", false, 4, 4),
        detail::build_token(Token_Type::OPENING_TAG, "c", "1", false, 5, 10, true),
        detail::build_token(Token_Type::CLOSING_TAG, "c", "", false, 10, 10, true)
    };

    detail::validate_tokens(parser, "{b/}x{c=1/}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_multiple_tags) {
    Text_Parser parser;

    std::vector<Token> tokens = {
        detail::build_token(Token_Type::OPENING_TAG, "a", "", false, 0, 2),
        detail::build_token(Token_Type::TEXT, "", "x", false, 3, 3),
        detail::build_token(Token_Type::CLOSING_TAG, "a", "", false, 4, 7),
        detail::build_token(Token_Type::OPENING_TAG, "b", "", false, 8, 10),
        detail::build_token(Token_Type::TEXT, "", "y", false, 11, 11),
        detail::build_token(Token_Type::CLOSING_TAG, "b", "", false, 12, 15)
    };

    detail::validate_tokens(parser, "{a}x{/a}{b}y{/b}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_unmatched_tags) {
    Text_Parser parser;

    std::vector<Token> tokens = {
        detail::build_token(Token_Type::OPENING_TAG, "a", "", true, 0, 2),
        detail::build_token(Token_Type::CLOSING_TAG, "b", "", true, 3, 6),
    };

    detail::validate_tokens(parser, "{a}{/b}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_multiple_unmatched_tags) {
    Text_Parser parser;

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

    detail::validate_tokens(parser, "{a}{b}{a}{/b}{a}{/a}{/b}{/a}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_escaped_tags) {
    Text_Parser parser;

    std::vector<Token> tokens = {
        detail::build_token(Token_Type::TEXT, "", "{b}hello{b} world{/b}", false, 0, 26),
    };

    detail::validate_tokens(parser, "{{b}}hello{{b}} world{{/b}}", tokens);
}

BOOST_AUTO_TEST_SUITE_END()
