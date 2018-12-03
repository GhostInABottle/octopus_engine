#include <boost/test/unit_test.hpp>
#include "../../include/text_parser.hpp"

namespace detail {
    std::string invalid_cases[] = {
        "a{",           // open brace at the end
        "a{/",          // close brace at the end
        "a}",           // unexpected closing tag
        "{b}a{/b=4}",   // equal sign in closing tag
        "{}",           // empty tag
        "{/}",          // empty tag
        "{a=}",         // empty value
        "{//}",         // unexpected character in tag
        "{/b/b}",       // unexpected character in tag
        "{{}}",         // unexpected character in tag
        "{a",           // tag was not closed
        "{/a",          // tag was not closed
        "aaa}"          // unexpected closing tag
    };

    void validate_tokens(const Text_Parser& parser, const std::string& text, std::vector<Token> expectedTokens) {
        std::vector<Token> actualTokens;

        BOOST_CHECK_NO_THROW(actualTokens = parser.parse(text));

        BOOST_CHECK_EQUAL(actualTokens.size(), expectedTokens.size());

        for (std::size_t i = 0; i < actualTokens.size(); ++i) {
            Token& actual = actualTokens[i], expected = expectedTokens[i];
            BOOST_CHECK_EQUAL(actual.type, expected.type);
            BOOST_CHECK_EQUAL(actual.tag, expected.tag);
            BOOST_CHECK_EQUAL(actual.value, expected.value);
            BOOST_CHECK_EQUAL(actual.unmatched, expected.unmatched);
            BOOST_CHECK_EQUAL(actual.start_index, expected.start_index);
            BOOST_CHECK_EQUAL(actual.end_index, expected.end_index);
        }
    }

    Token build_token(std::string type, std::string tag, std::string value, bool unmatched, int start, int end) {
        Token token;
        token.type = type;
        token.tag = tag;
        token.value = value;
        token.unmatched = unmatched;
        token.start_index = start;
        token.end_index = end;
        return token;
    }
}

BOOST_AUTO_TEST_SUITE(text_parser_tests)

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

    std::vector<Token> tokens;
    tokens.push_back(detail::build_token("text", "", "hello", false, 0, 4));

    detail::validate_tokens(parser, "hello", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_simple_tag) {
    Text_Parser parser;

    std::vector<Token> tokens;
    tokens.push_back(detail::build_token("opening_tag", "b", "", false, 0, 2));
    tokens.push_back(detail::build_token("text", "", "hello", false, 3, 7));
    tokens.push_back(detail::build_token("closing_tag", "b", "", false, 8, 11));

    detail::validate_tokens(parser, "{b}hello{/b}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_simple_tag_with_value) {
    Text_Parser parser;

    std::vector<Token> tokens;
    tokens.push_back(detail::build_token("opening_tag", "b", "c", false, 0, 4));
    tokens.push_back(detail::build_token("text", "", "hello", false, 5, 9));
    tokens.push_back(detail::build_token("closing_tag", "b", "", false, 10, 13));

    detail::validate_tokens(parser, "{b=c}hello{/b}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_simple_tag_with_empty_text) {
    Text_Parser parser;

    std::vector<Token> tokens;
    tokens.push_back(detail::build_token("opening_tag", "b", "", false, 0, 2));
    tokens.push_back(detail::build_token("closing_tag", "b", "", false, 3, 6));

    detail::validate_tokens(parser, "{b}{/b}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_multiple_tags) {
    Text_Parser parser;

    std::vector<Token> tokens;
    tokens.push_back(detail::build_token("opening_tag", "a", "", false, 0, 2));
    tokens.push_back(detail::build_token("text", "", "x", false, 3, 3));
    tokens.push_back(detail::build_token("closing_tag", "a", "", false, 4, 7));
    tokens.push_back(detail::build_token("opening_tag", "b", "", false, 8, 10));
    tokens.push_back(detail::build_token("text", "", "y", false, 11, 11));
    tokens.push_back(detail::build_token("closing_tag", "b", "", false, 12, 15));

    detail::validate_tokens(parser, "{a}x{/a}{b}y{/b}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_unmatched_tags) {
    Text_Parser parser;

    std::vector<Token> tokens;
    tokens.push_back(detail::build_token("opening_tag", "a", "", true, 0, 2));
    tokens.push_back(detail::build_token("closing_tag", "b", "", true, 3, 6));

    detail::validate_tokens(parser, "{a}{/b}", tokens);
}

BOOST_AUTO_TEST_CASE(text_parser_parses_multiple_unmatched_tags) {
    Text_Parser parser;

    std::vector<Token> tokens;
    tokens.push_back(detail::build_token("opening_tag", "a", "", true, 0, 2));
    tokens.push_back(detail::build_token("opening_tag", "b", "", false, 3, 5));
    tokens.push_back(detail::build_token("opening_tag", "a", "", false, 6, 8));
    tokens.push_back(detail::build_token("closing_tag", "b", "", false, 9, 12));
    tokens.push_back(detail::build_token("opening_tag", "a", "", false, 13, 15));
    tokens.push_back(detail::build_token("closing_tag", "a", "", false, 16, 19));
    tokens.push_back(detail::build_token("closing_tag", "b", "", true, 20, 23));
    tokens.push_back(detail::build_token("closing_tag", "a", "", false, 24, 27));

    detail::validate_tokens(parser, "{a}{b}{a}{/b}{a}{/a}{/b}{/a}", tokens);
}

BOOST_AUTO_TEST_SUITE_END()
