#include <unordered_map>
#include <boost/test/unit_test.hpp>
#include "../../include/utility/color.hpp"
#include "../../include/utility/direction.hpp"
#include "../../include/utility/math.hpp"
#include "../../include/utility/string.hpp"

BOOST_AUTO_TEST_SUITE(utility_tests)

namespace detail {
    inline void check_color(const xd::vec4& resulting_color, const xd::vec4& expected_color) {
        float epsilon = 0.01f;
        BOOST_CHECK_CLOSE(resulting_color.r, expected_color.r, epsilon);
        BOOST_CHECK_CLOSE(resulting_color.g, expected_color.g, epsilon);
        BOOST_CHECK_CLOSE(resulting_color.b, expected_color.b, epsilon);
    }
    std::unordered_map<std::string, xd::vec4> hex_to_color_map = {
        { "000000", xd::vec4(0.0f, 0.0f, 0.0f, 1.0f) },
        { "ffffff", xd::vec4(1.0f, 1.0f, 1.0f, 1.0f) },
        { "ca50f0", xd::vec4(0.79215686f, 0.31372549f, 0.94117647f, 1.0f) },
        { "f0ca50f0", xd::vec4(0.79215686f, 0.31372549f, 0.94117647f, 0.94117647f) },
    };
}

BOOST_AUTO_TEST_CASE(color_utility_hex_to_color) {
    BOOST_CHECK_THROW(hex_to_color(""), std::exception);
    BOOST_CHECK_THROW(hex_to_color("00FF00FF0"), std::exception);
    BOOST_CHECK_THROW(hex_to_color("REDRUM"), std::exception);
    for (auto& [hex, color] : detail::hex_to_color_map) {
        detail::check_color(hex_to_color(hex), color);
        detail::check_color(hex_to_color("#" + hex), color);
        detail::check_color(hex_to_color("#" + string_utilities::capitalize(hex)), color);
    }
}

BOOST_AUTO_TEST_CASE(color_utility_color_to_hex) {
    for (auto& [hex, color] : detail::hex_to_color_map) {
        BOOST_CHECK_EQUAL(color_to_hex(color), hex);
        if (hex.size() == 6) {
            BOOST_CHECK_EQUAL(color_to_hex(color, true), "ff" + hex);
        }
    }
}

BOOST_AUTO_TEST_CASE(math_utility_check_close) {
    BOOST_CHECK(check_close(4.56f, 4.56f));
    BOOST_CHECK(check_close(-54.566780f, -54.566781f));
    BOOST_CHECK(check_close(-54.566780f, -54.566781f, 0.0001f));
}

BOOST_AUTO_TEST_CASE(direction_utility_is_diagonal) {
    BOOST_CHECK(is_diagonal(Direction::LEFT | Direction::UP));
    BOOST_CHECK(is_diagonal(Direction::DOWN | Direction::RIGHT));
    BOOST_CHECK(!is_diagonal(Direction::LEFT));
    BOOST_CHECK(!is_diagonal(Direction::UP));
}

BOOST_AUTO_TEST_CASE(direction_utility_direction_to_string) {
    BOOST_CHECK_EQUAL(direction_to_string(Direction::NONE), "");
    BOOST_CHECK_EQUAL(direction_to_string(Direction::UP), "Up");
    BOOST_CHECK_EQUAL(direction_to_string(Direction::RIGHT), "Right");
    BOOST_CHECK_EQUAL(direction_to_string(Direction::UP | Direction::RIGHT), "Up|Right");
    BOOST_CHECK_EQUAL(direction_to_string(Direction::DOWN), "Down");
    BOOST_CHECK_EQUAL(direction_to_string(Direction::DOWN | Direction::RIGHT), "Down|Right");
    BOOST_CHECK_EQUAL(direction_to_string(Direction::UP | Direction::DOWN | Direction::RIGHT), "Up|Down|Right");
    BOOST_CHECK_EQUAL(direction_to_string(Direction::LEFT), "Left");
    BOOST_CHECK_EQUAL(direction_to_string(Direction::UP | Direction::LEFT), "Up|Left");
    BOOST_CHECK_EQUAL(direction_to_string(Direction::LEFT | Direction::RIGHT), "Left|Right");
    BOOST_CHECK_EQUAL(direction_to_string(Direction::UP | Direction::LEFT | Direction::RIGHT), "Up|Left|Right");
    BOOST_CHECK_EQUAL(direction_to_string(Direction::DOWN | Direction::LEFT), "Down|Left");
    BOOST_CHECK_EQUAL(direction_to_string(Direction::UP | Direction::DOWN | Direction::LEFT), "Up|Down|Left");
    BOOST_CHECK_EQUAL(direction_to_string(Direction::DOWN | Direction::LEFT | Direction::RIGHT), "Down|Left|Right");
    BOOST_CHECK_EQUAL(direction_to_string(Direction::UP | Direction::DOWN | Direction::LEFT | Direction::RIGHT), "Up|Down|Left|Right");
}

BOOST_AUTO_TEST_CASE(direction_utility_string_to_direction) {
    BOOST_CHECK_EQUAL(string_to_direction(""), Direction::NONE);
    BOOST_CHECK_EQUAL(string_to_direction("Up"), Direction::UP);
    BOOST_CHECK_EQUAL(string_to_direction("Left"), Direction::LEFT);
    BOOST_CHECK_EQUAL(string_to_direction("Down"), Direction::DOWN);
    BOOST_CHECK_EQUAL(string_to_direction("Right"), Direction::RIGHT);
    BOOST_CHECK_EQUAL(string_to_direction("Up|Left"), Direction::UP | Direction::LEFT);
    BOOST_CHECK_EQUAL(string_to_direction("Down|Right"), Direction::DOWN | Direction::RIGHT);
    BOOST_CHECK_EQUAL(string_to_direction("Right|Right|Up"), Direction::UP | Direction::RIGHT);
    BOOST_CHECK_EQUAL(string_to_direction("Up|Down|Left|Right"), Direction::UP | Direction::DOWN | Direction::LEFT | Direction::RIGHT);
    BOOST_CHECK_EQUAL(string_to_direction("Wrong"), Direction::NONE);
    BOOST_CHECK_EQUAL(string_to_direction("Wrong|Right"), Direction::RIGHT);
}

BOOST_AUTO_TEST_CASE(direction_utility_diagonal_to_four_directions) {
    BOOST_CHECK_EQUAL(diagonal_to_four_directions(Direction::LEFT | Direction::UP), Direction::UP);
    BOOST_CHECK_EQUAL(diagonal_to_four_directions(Direction::DOWN | Direction::RIGHT), Direction::DOWN);
    BOOST_CHECK_EQUAL(diagonal_to_four_directions(Direction::UP | Direction::DOWN), Direction::UP);
    BOOST_CHECK_EQUAL(diagonal_to_four_directions(Direction::LEFT | Direction::RIGHT), Direction::LEFT);
    BOOST_CHECK_EQUAL(diagonal_to_four_directions(Direction::RIGHT), Direction::RIGHT);
    BOOST_CHECK_EQUAL(diagonal_to_four_directions(Direction::UP), Direction::UP);
    BOOST_CHECK_EQUAL(diagonal_to_four_directions(Direction::DOWN), Direction::DOWN);
    BOOST_CHECK_EQUAL(diagonal_to_four_directions(Direction::LEFT), Direction::LEFT);
    BOOST_CHECK_EQUAL(diagonal_to_four_directions(Direction::UP | Direction::DOWN | Direction::LEFT | Direction::RIGHT), Direction::UP);
}

BOOST_AUTO_TEST_SUITE_END()
