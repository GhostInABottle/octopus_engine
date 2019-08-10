#include <boost/test/unit_test.hpp>
#include "../../include/xd/asset_manager.hpp"
#include "../../include/vendor/rapidxml.hpp"
#include "../../include/sprite_data.hpp"
#include "../../include/map.hpp"
#include "../../include/game.hpp"

BOOST_AUTO_TEST_SUITE(sprite_data_tests)

BOOST_AUTO_TEST_CASE(sprite_data_load_file) {
    xd::asset_manager manager;
    auto sprite_data = Sprite_Data::load(manager, "sprite.spr");
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
          <Pose Name=\"Cool Pose\" Repeats=\"3\" Require-Completion=\"true\" > \
            <Bounding-Box X=\"1\" Y=\"2\" Width=\"3\" Height=\"4\" /> \
            <Frame Duration=\"200\" X-Mag=\"2.5\" Y-Mag=\"4.0\" Angle=\"10\" Opacity=\"0.5\" Tween=\"true\"> \
              <Rectangle X=\"1\" Y=\"2\" Width=\"3\" Height=\"4\" /> \
            </Frame> \
          </Pose> \
        </Sprite>";
    rapidxml::xml_document<> doc;
    doc.parse<0>(text);
    rapidxml::xml_node<>* node = doc.first_node("Sprite");
    BOOST_CHECK(node);
    xd::asset_manager manager;
    auto sprite_data = Sprite_Data::load(manager, *node);
    Pose& main_pose = sprite_data->poses[0];
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
    BOOST_CHECK_EQUAL(main_pose.frames[0].angle, 10);
    float epsilon = 0.01f;
    BOOST_CHECK_CLOSE(main_pose.frames[0].magnification.x, 2.5f, epsilon);
    BOOST_CHECK_CLOSE(main_pose.frames[0].magnification.y, 4.0f, epsilon);
    BOOST_CHECK_CLOSE(main_pose.frames[0].opacity, 0.5f, epsilon);
}

BOOST_AUTO_TEST_SUITE_END()
