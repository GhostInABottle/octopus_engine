#include <boost/test/unit_test.hpp>
#include "../../include/utility.hpp"
#include "../../include/direction_utilities.hpp"

BOOST_AUTO_TEST_SUITE(utility_tests)

namespace detail {
    inline void check_color(const xd::vec4& resulting_color, const xd::vec4& expected_color) {
        float epsilon = 0.01f;
        BOOST_CHECK_CLOSE(resulting_color.r, expected_color.r, epsilon);
        BOOST_CHECK_CLOSE(resulting_color.g, expected_color.g, epsilon);
        BOOST_CHECK_CLOSE(resulting_color.b, expected_color.b, epsilon);
    }
}

BOOST_AUTO_TEST_CASE(utility_hex_to_color) {
    BOOST_CHECK_THROW(hex_to_color(""), std::exception);
    BOOST_CHECK_THROW(hex_to_color("00FF00FF0"), std::exception);
    BOOST_CHECK_THROW(hex_to_color("REDRUM"), std::exception);
    detail::check_color(hex_to_color("000000"), xd::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    detail::check_color(hex_to_color("FFFFFF"), xd::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    detail::check_color(hex_to_color("CA50F0"), xd::vec4(0.79215f, 0.31372f, 0.94117f, 1.0f));
    detail::check_color(hex_to_color("#F0CA50F0"), xd::vec4(0.79215f, 0.31372f, 0.94117f, 0.94117f));
}

BOOST_AUTO_TEST_CASE(utility_check_close) {
    BOOST_CHECK(check_close(4.56f, 4.56f));
    BOOST_CHECK(check_close(-54.566780f, -54.566781f));
    BOOST_CHECK(check_close(-54.566780f, -54.566781f, 0.0001f));
}

BOOST_AUTO_TEST_CASE(utility_is_diagonal) {
    BOOST_CHECK(is_diagonal(Direction::LEFT | Direction::UP));
    BOOST_CHECK(is_diagonal(Direction::DOWN | Direction::RIGHT));
    BOOST_CHECK(!is_diagonal(Direction::LEFT));
    BOOST_CHECK(!is_diagonal(Direction::UP));
}

BOOST_AUTO_TEST_SUITE_END()
