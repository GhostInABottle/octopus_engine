#include <unordered_map>
#include <boost/test/unit_test.hpp>
#include "../../include/utility/color.hpp"
#include "../../include/utility/direction.hpp"
#include "../../include/utility/math.hpp"
#include "../../include/utility/string.hpp"

BOOST_AUTO_TEST_SUITE(utility_tests)

namespace detail {
    static void check_color(const xd::vec4& resulting_color, const xd::vec4& expected_color) {
        float epsilon = 0.01f;
        BOOST_CHECK_CLOSE(resulting_color.r, expected_color.r, epsilon);
        BOOST_CHECK_CLOSE(resulting_color.g, expected_color.g, epsilon);
        BOOST_CHECK_CLOSE(resulting_color.b, expected_color.b, epsilon);
    }
    static std::unordered_map<std::string, xd::vec4> hex_to_color_map = {
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

BOOST_AUTO_TEST_CASE(xd_types_rect_intersects) {
    xd::rect r{1, 2, 5, 4 };
    // Valid cases
    BOOST_TEST(r.intersects(r)); // equal
    BOOST_TEST(r.intersects(xd::rect{ 2, 3, 3, 2})); // contained
    BOOST_TEST(r.intersects(xd::rect{ 0, 3, 2, 2 })); // left
    BOOST_TEST(r.intersects(xd::rect{ 2, 1, 2, 2 })); // above
    BOOST_TEST(r.intersects(xd::rect{ 5, 3, 2, 2 })); // right
    BOOST_TEST(r.intersects(xd::rect{ 2, 5, 2, 2 })); // below
    BOOST_TEST(r.intersects(xd::rect{ 0, 1, 2, 2 })); // left/above
    BOOST_TEST(r.intersects(xd::rect{ 0, 5, 2, 2 })); // left/below
    BOOST_TEST(r.intersects(xd::rect{ 5, 1, 2, 2 })); // right/above
    BOOST_TEST(r.intersects(xd::rect{ 5, 5, 2, 2 })); // right/below

    // Invalid cases
    BOOST_TEST(!r.intersects(xd::rect{ 7, 7, 1, 1 })); // outside
    BOOST_TEST(!r.intersects(xd::rect{ 0, 3, 1, 1 })); // touching left
    BOOST_TEST(!r.intersects(xd::rect{ 2, 1, 1, 1 })); // touching above
    BOOST_TEST(!r.intersects(xd::rect{ 6, 3, 1, 1 })); // touching right
    BOOST_TEST(!r.intersects(xd::rect{ 2, 6, 1, 1 })); // touching below
    BOOST_TEST(!r.intersects(xd::rect{ 0, 1, 1, 1 })); // touching left/above
    BOOST_TEST(!r.intersects(xd::rect{ 0, 6, 1, 1 })); // touching left/below
    BOOST_TEST(!r.intersects(xd::rect{ 6, 1, 1, 1 })); // touching right/above
    BOOST_TEST(!r.intersects(xd::rect{ 6, 6, 1, 1 })); // touching right/below
}

BOOST_AUTO_TEST_CASE(xd_types_rect_touches) {
    xd::rect r{ 1, 2, 5, 4 };
    // Valid cases
    BOOST_TEST(r.touches(xd::rect{ 0, 3, 1, 1 })); // touching left
    BOOST_TEST(r.touches(xd::rect{ 2, 1, 1, 1 })); // touching above
    BOOST_TEST(r.touches(xd::rect{ 6, 3, 1, 1 })); // touching right
    BOOST_TEST(r.touches(xd::rect{ 2, 6, 1, 1 })); // touching below
    BOOST_TEST(r.touches(xd::rect{ 0, 1, 1, 1 })); // touching left/above
    BOOST_TEST(r.touches(xd::rect{ 0, 6, 1, 1 })); // touching left/below
    BOOST_TEST(r.touches(xd::rect{ 6, 1, 1, 1 })); // touching right/above
    BOOST_TEST(r.touches(xd::rect{ 6, 6, 1, 1 })); // touching right/below

    // Invalid cases
    BOOST_TEST(!r.touches(r)); // equal
    BOOST_TEST(!r.touches(xd::rect{ 7, 7, 1, 1 })); // outside
    BOOST_TEST(!r.touches(xd::rect{ 2, 3, 3, 2 })); // contained
    BOOST_TEST(!r.touches(xd::rect{ 0, 3, 2, 2 })); // intersects left
    BOOST_TEST(!r.touches(xd::rect{ 2, 1, 2, 2 })); // intersects above
    BOOST_TEST(!r.touches(xd::rect{ 5, 3, 2, 2 })); // intersects right
    BOOST_TEST(!r.touches(xd::rect{ 2, 5, 2, 2 })); // intersects below
    BOOST_TEST(!r.touches(xd::rect{ 0, 1, 2, 2 })); // intersects left/above
    BOOST_TEST(!r.touches(xd::rect{ 0, 5, 2, 2 })); // intersects left/below
    BOOST_TEST(!r.touches(xd::rect{ 5, 1, 2, 2 })); // intersects right/above
    BOOST_TEST(!r.touches(xd::rect{ 5, 5, 2, 2 })); // intersects right/below
}

BOOST_AUTO_TEST_CASE(xd_types_rect_contains) {
    xd::rect r{ 1, 2, 5, 4 };
    // Valid cases
    BOOST_TEST(r.contains(2, 3)); // contained left
    BOOST_TEST(r.contains(2, 3)); // contained above
    BOOST_TEST(r.contains(5, 3)); // contained right
    BOOST_TEST(r.contains(2, 5)); // contained below
    BOOST_TEST(r.contains(2, 3)); // contained left/above
    BOOST_TEST(r.contains(2, 5)); // contained left/below
    BOOST_TEST(r.contains(5, 3)); // contained right/above
    BOOST_TEST(r.contains(5, 5)); // contained right/below
    BOOST_TEST(r.contains(1, 3)); // touching left
    BOOST_TEST(r.contains(2, 2)); // touching above
    BOOST_TEST(r.contains(6, 3)); // touching right
    BOOST_TEST(r.contains(2, 6)); // touching below
    BOOST_TEST(r.contains(1, 2)); // touching left/above
    BOOST_TEST(r.contains(1, 6)); // touching left/below
    BOOST_TEST(r.contains(6, 2)); // touching right/above
    BOOST_TEST(r.contains(6, 6)); // touching right/below

    // Invalid cases
    BOOST_TEST(!r.contains(0, 2)); // outside left
    BOOST_TEST(!r.contains(2, 1)); // outside above
    BOOST_TEST(!r.contains(7, 3)); // outside right
    BOOST_TEST(!r.contains(2, 7)); // outside below
    BOOST_TEST(!r.contains(0, 1)); // outside left/above
    BOOST_TEST(!r.contains(0, 7)); // outside left/below
    BOOST_TEST(!r.contains(7, 1)); // outside right/above
    BOOST_TEST(!r.contains(7, 7)); // outside right/below
}


BOOST_AUTO_TEST_CASE(xd_types_circle_intersects_circle) {
    xd::circle c{ 4, 4, 2 };
    // Valid cases
    BOOST_TEST(c.intersects(c)); // equal
    BOOST_TEST(c.intersects(xd::circle{ 4, 4, 1 })); // contained
    BOOST_TEST(c.intersects(xd::circle{ 1, 4, 2 })); // left
    BOOST_TEST(c.intersects(xd::circle{ 4, 1, 2 })); // above
    BOOST_TEST(c.intersects(xd::circle{ 7, 4, 2 })); // right
    BOOST_TEST(c.intersects(xd::circle{ 4, 7, 2 })); // below
    BOOST_TEST(c.intersects(xd::circle{ 1, 2, 2 })); // left/above
    BOOST_TEST(c.intersects(xd::circle{ 1, 6, 2 })); // left/below
    BOOST_TEST(c.intersects(xd::circle{ 6, 2, 2 })); // right/above
    BOOST_TEST(c.intersects(xd::circle{ 6, 6, 2 })); // right/below

    // Invalid cases
    BOOST_TEST(!c.intersects(xd::circle{ 8, 8, 1 })); // outside
    BOOST_TEST(!c.intersects(xd::circle{ 0, 4, 1 })); // touching left
    BOOST_TEST(!c.intersects(xd::circle{ 4, 1, 1 })); // touching above
    BOOST_TEST(!c.intersects(xd::circle{ 7, 4, 1 })); // touching right
    BOOST_TEST(!c.intersects(xd::circle{ 4, 7, 1 })); // touching below
    BOOST_TEST(!c.intersects(xd::circle{ 1, 1, 2 })); // touching left/above
    BOOST_TEST(!c.intersects(xd::circle{ 1, 7, 2 })); // touching left/below
    BOOST_TEST(!c.intersects(xd::circle{ 7, 1, 2 })); // touching right/above
    BOOST_TEST(!c.intersects(xd::circle{ 7, 7, 2 })); // touching right/below
}

BOOST_AUTO_TEST_CASE(xd_types_circle_intersects_rect) {
    xd::circle c{ 4, 4, 2 };
    // Valid cases
    BOOST_TEST(c.intersects(static_cast<xd::rect>(c))); // equal
    BOOST_TEST(c.intersects(xd::rect{ 3, 3, 1, 1 })); // contained
    BOOST_TEST(c.intersects(xd::rect{ 1, 3, 2, 2 })); // left
    BOOST_TEST(c.intersects(xd::rect{ 3, 1, 2, 2 })); // above
    BOOST_TEST(c.intersects(xd::rect{ 5, 3, 2, 2 })); // right
    BOOST_TEST(c.intersects(xd::rect{ 3, 5, 2, 2 })); // below
    BOOST_TEST(c.intersects(xd::rect{ 1, 2, 2, 2 })); // left/above
    BOOST_TEST(c.intersects(xd::rect{ 1, 5, 2, 2 })); // left/below
    BOOST_TEST(c.intersects(xd::rect{ 5, 1, 2, 2 })); // right/above
    BOOST_TEST(c.intersects(xd::rect{ 5, 5, 2, 2 })); // right/below

    // Invalid cases
    BOOST_TEST(!c.intersects(xd::rect{ 7, 7, 1, 1 })); // outside
    BOOST_TEST(!c.intersects(xd::rect{ 1, 3, 1, 2 })); // touching left
    BOOST_TEST(!c.intersects(xd::rect{ 3, 1, 2, 1 })); // touching above
    BOOST_TEST(!c.intersects(xd::rect{ 6, 3, 1, 1 })); // touching right
    BOOST_TEST(!c.intersects(xd::rect{ 3, 6, 1, 1 })); // touching below
    BOOST_TEST(!c.intersects(xd::rect{ 2, 1, 1, 1 })); // outside left/above
    BOOST_TEST(!c.intersects(xd::rect{ 2, 6, 1, 1 })); // outside left/below
    BOOST_TEST(!c.intersects(xd::rect{ 6, 1, 1, 1 })); // outside right/above
    BOOST_TEST(!c.intersects(xd::rect{ 6, 6, 1, 1 })); // outside right/below
}

BOOST_AUTO_TEST_CASE(xd_types_circle_touches) {
    xd::circle c{ 4, 4, 2 };
    // Valid cases
    BOOST_TEST(c.touches(xd::circle{ 1, 4, 1 })); // touching left
    BOOST_TEST(c.touches(xd::circle{ 4, 1, 1 })); // touching above
    BOOST_TEST(c.touches(xd::circle{ 7, 4, 1 })); // touching right
    BOOST_TEST(c.touches(xd::circle{ 4, 7, 1 })); // touching below

    // Invalid cases
    BOOST_TEST(!c.touches(c)); // equal
    BOOST_TEST(!c.touches(xd::circle{ 4, 4, 1 })); // contained
    BOOST_TEST(!c.touches(xd::circle{ 8, 8, 1 })); // outside
    BOOST_TEST(!c.touches(xd::circle{ 1, 4, 2 })); // left
    BOOST_TEST(!c.touches(xd::circle{ 4, 1, 2 })); // above
    BOOST_TEST(!c.touches(xd::circle{ 7, 4, 2 })); // right
    BOOST_TEST(!c.touches(xd::circle{ 4, 7, 2 })); // below
}

BOOST_AUTO_TEST_CASE(xd_types_circle_contains) {
    xd::circle c{ 4, 4, 2 };

    // Valid cases
    BOOST_TEST(c.contains(2.1f, 4)); // contained left
    BOOST_TEST(c.contains(4, 2.1f)); // contained above
    BOOST_TEST(c.contains(5.9f, 4)); // contained right
    BOOST_TEST(c.contains(4, 5.9f)); // contained below
    BOOST_TEST(c.contains(2, 4)); // touching left
    BOOST_TEST(c.contains(4, 2)); // touching above
    BOOST_TEST(c.contains(6, 4)); // touching right
    BOOST_TEST(c.contains(4, 6)); // touching below

    // Invalid cases
    BOOST_TEST(!c.contains(1.9f, 4)); // outside left
    BOOST_TEST(!c.contains(4, 1.9f)); // outside above
    BOOST_TEST(!c.contains(6.1f, 4)); // outside right
    BOOST_TEST(!c.contains(4, 6.1f)); // outside below
    BOOST_TEST(!c.contains(2, 3)); // outside left/above
    BOOST_TEST(!c.contains(2, 5)); // outside left/below
    BOOST_TEST(!c.contains(6, 3)); // outside right/above
    BOOST_TEST(!c.contains(6, 6)); // outside right/below
}

BOOST_AUTO_TEST_SUITE_END()
