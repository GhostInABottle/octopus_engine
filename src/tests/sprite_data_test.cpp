#include "../../include/game.hpp"
#include "../../include/map/map.hpp"
#include "../../include/sprite_data.hpp"
#include "../../include/filesystem/user_data_folder.hpp"
#include "../../include/vendor/rapidxml.hpp"
#include "../../include/xd/asset_manager.hpp"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(sprite_data_tests)

BOOST_AUTO_TEST_CASE(sprite_data_load_file) {
    User_Data_Folder::parse_default_config();
    xd::asset_manager manager;
    auto sprite_data = Sprite_Data::load("sprite.spr", manager, nullptr, channel_group_type::sound);
    Pose& main_pose = sprite_data->poses[0];
    BOOST_CHECK_EQUAL(main_pose.duration, 180);
    BOOST_CHECK_EQUAL(main_pose.repeats, -1);
    BOOST_CHECK_EQUAL(main_pose.bounding_box.x, 0);
    BOOST_CHECK_EQUAL(main_pose.frames[0].rectangle.w, 21);
    BOOST_CHECK_EQUAL(main_pose.frames[1].tween_frame, false);
    float epsilon = 0.01f;
    BOOST_CHECK_CLOSE(main_pose.frames[0].magnification.y, 1.0f, epsilon);
}

BOOST_AUTO_TEST_CASE(sprite_data_load) {
    char text[] =
        "<?xml version=\"1.0\"?> \
        <Sprite Image=\"../data/player.png\"> \
          <Pose Repeats=\"3\" Require-Completion=\"true\" > \
            <Tag Key=\"Name\" Value=\"Cool Pose\" /> \
            <Tag Key=\"State\" Value=\"St\" /> \
            <Tag Key=\"Direction\" Value=\"Up\" /> \
            <Completion-Frame Index=\"1\" /> \
            <Bounding-Box X=\"1\" Y=\"2\" Width=\"3\" Height=\"4\" /> \
            <Frame Duration=\"200\" X-Mag=\"2.5\" Y-Mag=\"4.0\" Angle=\"10\" Opacity=\"0.5\" Tween=\"true\"> \
              <Rectangle X=\"1\" Y=\"2\" Width=\"3\" Height=\"4\" /> \
            </Frame> \
          </Pose> \
          <Pose Duration=\"10\" > \
            <Tag Key=\"Name\" Value=\"Circle Pose\"/> \
            <Bounding-Circle X=\"5\" Y=\"7\" Radius=\"4\" /> \
            <Frame Max-Duration=\"350\" Marker=\"ABC\"> \
              <Rectangle X=\"11\" Y=\"12\" Width=\"13\" Height=\"14\" /> \
            </Frame> \
          </Pose> \
        </Sprite>";
    auto doc = std::make_unique<rapidxml::xml_document<>>();
    doc->parse<0>(text);
    auto node = doc->first_node("Sprite");
    BOOST_CHECK(node);

    User_Data_Folder::parse_default_config();
    xd::asset_manager manager;
    auto sprite_data = Sprite_Data::load(*node, "filename", manager, nullptr, channel_group_type::sound);

    BOOST_CHECK_EQUAL(sprite_data->filename, "filename");
    Pose& main_pose = sprite_data->poses[0];
    BOOST_CHECK_EQUAL(main_pose.name, "COOL POSE");
    BOOST_CHECK_EQUAL(main_pose.state, "ST");
    BOOST_CHECK_EQUAL(static_cast<int>(main_pose.direction), static_cast<int>(Direction::UP));
    BOOST_CHECK_EQUAL(main_pose.duration, 100);
    BOOST_CHECK_EQUAL(main_pose.repeats, 3);
    BOOST_CHECK_EQUAL(main_pose.require_completion, true);
    BOOST_CHECK(main_pose.completion_frames);
    BOOST_CHECK_EQUAL(main_pose.completion_frames.value().at(0), 0);
    BOOST_CHECK_EQUAL(main_pose.bounding_box.x, 1);
    BOOST_CHECK_EQUAL(main_pose.bounding_box.y, 2);
    BOOST_CHECK_EQUAL(main_pose.bounding_box.w, 3);
    BOOST_CHECK_EQUAL(main_pose.bounding_box.h, 4);
    BOOST_CHECK_EQUAL(main_pose.frames[0].rectangle.x, 1);
    BOOST_CHECK_EQUAL(main_pose.frames[0].rectangle.y, 2);
    BOOST_CHECK_EQUAL(main_pose.frames[0].rectangle.w, 3);
    BOOST_CHECK_EQUAL(main_pose.frames[0].rectangle.h, 4);
    BOOST_CHECK_EQUAL(main_pose.frames[0].tween_frame, true);
    BOOST_CHECK_EQUAL(main_pose.frames[0].duration, 200);
    BOOST_CHECK_EQUAL(main_pose.frames[0].max_duration, -1);
    BOOST_CHECK_EQUAL(main_pose.frames[0].angle, 10);
    float epsilon = 0.01f;
    BOOST_CHECK_CLOSE(main_pose.frames[0].magnification.x, 2.5f, epsilon);
    BOOST_CHECK_CLOSE(main_pose.frames[0].magnification.y, 4.0f, epsilon);
    BOOST_CHECK_CLOSE(main_pose.frames[0].opacity, 0.5f, epsilon);

    Pose& circle_pose = sprite_data->poses[1];
    BOOST_CHECK_EQUAL(circle_pose.name, "CIRCLE POSE");
    BOOST_CHECK_EQUAL(circle_pose.duration, 10);
    BOOST_CHECK_EQUAL(circle_pose.repeats, -1);
    BOOST_CHECK_EQUAL(circle_pose.require_completion, false);
    BOOST_CHECK(!circle_pose.completion_frames);
    BOOST_CHECK(circle_pose.bounding_circle.has_value());
    BOOST_CHECK_EQUAL(circle_pose.bounding_circle->x, 5);
    BOOST_CHECK_EQUAL(circle_pose.bounding_circle->y, 7);
    BOOST_CHECK_EQUAL(circle_pose.bounding_circle->radius, 4);
    BOOST_CHECK_EQUAL(circle_pose.bounding_box.x, 1);
    BOOST_CHECK_EQUAL(circle_pose.bounding_box.y, 3);
    BOOST_CHECK_EQUAL(circle_pose.bounding_box.w, 8);
    BOOST_CHECK_EQUAL(circle_pose.bounding_box.h, 8);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].rectangle.x, 11);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].rectangle.y, 12);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].rectangle.w, 13);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].rectangle.h, 14);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].tween_frame, false);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].duration, -1);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].max_duration, 350);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].marker, "ABC");
    BOOST_CHECK_EQUAL(circle_pose.frames[0].angle, 0);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].magnification.x, 1.0f);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].magnification.y, 1.0f);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].opacity, 1.0f);
}

BOOST_AUTO_TEST_CASE(sprite_data_load_compact) {
    char text[] =
        "<?xml version=\"1.0\"?> \
        <Sprite Image=\"../data/player.png\"> \
          <Pose Name=\"Cool Pose\" State=\"St\" Direction=\"UP\" Repeats=\"3\" Require-Completion=\"true\"> \
            <Bounding-Box X=\"1\" Y=\"2\" Width=\"3\" Height=\"4\" /> \
            <Frame X=\"1\" Y=\"2\" Width=\"3\" Height=\"4\" Duration=\"200\" X-Mag=\"2.5\" Y-Mag=\"4.0\" Angle=\"10\" Opacity=\"0.5\" Tween=\"true\" /> \
          </Pose> \
          <Pose Name=\"Circle Pose\" Duration=\"10\"> \
            <Bounding-Circle X=\"5\" Y=\"7\" Radius=\"4\" /> \
            <Frame X=\"11\" Y=\"12\" Width=\"13\" Height=\"14\" Max-Duration=\"350\" Marker=\"ABC\" /> \
          </Pose> \
        </Sprite>";
    auto doc = std::make_unique<rapidxml::xml_document<>>();
    doc->parse<0>(text);
    auto node = doc->first_node("Sprite");
    BOOST_CHECK(node);

    User_Data_Folder::parse_default_config();
    xd::asset_manager manager;
    auto sprite_data = Sprite_Data::load(*node, "filename", manager, nullptr, channel_group_type::sound);

    BOOST_CHECK_EQUAL(sprite_data->filename, "filename");
    Pose& main_pose = sprite_data->poses[0];
    BOOST_CHECK_EQUAL(main_pose.name, "COOL POSE");
    BOOST_CHECK_EQUAL(main_pose.state, "ST");
    BOOST_CHECK_EQUAL(static_cast<int>(main_pose.direction), static_cast<int>(Direction::UP));
    BOOST_CHECK_EQUAL(main_pose.duration, 100);
    BOOST_CHECK_EQUAL(main_pose.repeats, 3);
    BOOST_CHECK_EQUAL(main_pose.require_completion, true);
    BOOST_CHECK_EQUAL(main_pose.bounding_box.x, 1);
    BOOST_CHECK_EQUAL(main_pose.bounding_box.y, 2);
    BOOST_CHECK_EQUAL(main_pose.bounding_box.w, 3);
    BOOST_CHECK_EQUAL(main_pose.bounding_box.h, 4);
    BOOST_CHECK_EQUAL(main_pose.frames[0].rectangle.x, 1);
    BOOST_CHECK_EQUAL(main_pose.frames[0].rectangle.y, 2);
    BOOST_CHECK_EQUAL(main_pose.frames[0].rectangle.w, 3);
    BOOST_CHECK_EQUAL(main_pose.frames[0].rectangle.h, 4);
    BOOST_CHECK_EQUAL(main_pose.frames[0].tween_frame, true);
    BOOST_CHECK_EQUAL(main_pose.frames[0].duration, 200);
    BOOST_CHECK_EQUAL(main_pose.frames[0].max_duration, -1);
    BOOST_CHECK_EQUAL(main_pose.frames[0].angle, 10);
    float epsilon = 0.01f;
    BOOST_CHECK_CLOSE(main_pose.frames[0].magnification.x, 2.5f, epsilon);
    BOOST_CHECK_CLOSE(main_pose.frames[0].magnification.y, 4.0f, epsilon);
    BOOST_CHECK_CLOSE(main_pose.frames[0].opacity, 0.5f, epsilon);

    Pose& circle_pose = sprite_data->poses[1];
    BOOST_CHECK_EQUAL(circle_pose.name, "CIRCLE POSE");
    BOOST_CHECK_EQUAL(circle_pose.duration, 10);
    BOOST_CHECK_EQUAL(circle_pose.repeats, -1);
    BOOST_CHECK_EQUAL(circle_pose.require_completion, false);
    BOOST_CHECK(circle_pose.bounding_circle.has_value());
    BOOST_CHECK_EQUAL(circle_pose.bounding_circle->x, 5);
    BOOST_CHECK_EQUAL(circle_pose.bounding_circle->y, 7);
    BOOST_CHECK_EQUAL(circle_pose.bounding_circle->radius, 4);
    BOOST_CHECK_EQUAL(circle_pose.bounding_box.x, 1);
    BOOST_CHECK_EQUAL(circle_pose.bounding_box.y, 3);
    BOOST_CHECK_EQUAL(circle_pose.bounding_box.w, 8);
    BOOST_CHECK_EQUAL(circle_pose.bounding_box.h, 8);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].rectangle.x, 11);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].rectangle.y, 12);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].rectangle.w, 13);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].rectangle.h, 14);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].tween_frame, false);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].duration, -1);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].max_duration, 350);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].marker, "ABC");
    BOOST_CHECK_EQUAL(circle_pose.frames[0].angle, 0);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].magnification.x, 1.0f);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].magnification.y, 1.0f);
    BOOST_CHECK_EQUAL(circle_pose.frames[0].opacity, 1.0f);
}

BOOST_AUTO_TEST_SUITE_END()
