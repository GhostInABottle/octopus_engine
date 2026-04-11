#include "game_fixture.hpp"
#include "../map/map_object.hpp"
#include "../utility/direction.hpp"
#include "../utility/color.hpp"
#include "../xd/asset_manager.hpp"
#include "../vendor/rapidxml.hpp"
#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(map_tests, Game_Fixture)

BOOST_AUTO_TEST_CASE(map_object_load) {
    char text[] =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
         <object id=\"3\" name=\"stairs\" type=\"area\" x=\"312\" y=\"112\" width=\"63\" height=\"122\" gid=\"5\" visible=\"0\"> \
          <properties> \
           <property name=\"sprite\" value=\"data/sprite.spr\"/> \
           <property name=\"direction\" value=\"UP|LEFT\"/> \
           <property name=\"pose\" value=\"pose name\"/> \
           <property name=\"state\" value=\"state name\"/> \
           <property name=\"face-state\" value=\"face state name\"/> \
           <property name=\"walk-state\" value=\"walk state name\"/> \
           <property name=\"speed\" value=\"5.5\"/> \
           <property name=\"movement-speed\" value=\"4.5\"/> \
           <property name=\"animation-speed\" value=\"3.5\"/> \
           <property name=\"color\" value=\"red\"/> \
           <property name=\"opacity\" value=\"0.5\"/> \
           <property name=\"script-context\" value=\"global\"/> \
           <property name=\"script\" value=\"trigger script\"/> \
           <property name=\"leave-script\" value=\"leave script\"/> \
           <property name=\"touch-script\" value=\"touch script\"/> \
           <property name=\"passthrough\" value=\"true\"/> \
           <property name=\"passthrough-type\" value=\"initiator\"/> \
           <property name=\"override-tile-collision\" value=\"true\"/> \
           <property name=\"collision-priority\" value=\"33\"/> \
           <property name=\"proximity-distance\" value=\"22\"/> \
           <property name=\"use-layer-color\" value=\"false\"/> \
           <property name=\"outlined\" value=\"script, solid, touched, proximate\"/> \
           <property name=\"outlined-object\" value=\"8\"/> \
           <property name=\"outline-color\" value=\"#115511\"/> \
           <property name=\"draw-order\" value=\"above\"/> \
           <property name=\"sfx-attenuation\" value=\"true\"/> \
          </properties> \
         </object>";

    auto doc = std::make_unique<rapidxml::xml_document<>>();
    doc->parse<0>(text);
    auto node = doc->first_node("object");
    BOOST_CHECK(node);
    xd::asset_manager manager;
    auto object = Map_Object::load(*node, *game, manager);

    constexpr float epsilon = 0.001f;
    BOOST_CHECK_EQUAL(object->get_id(), 3);
    BOOST_CHECK_EQUAL(object->get_name(), "STAIRS");
    BOOST_CHECK_EQUAL(object->get_type(), "area");
    BOOST_CHECK_EQUAL(object->get_x(), 312);
    BOOST_CHECK_EQUAL(object->get_y(), 112);
    BOOST_CHECK_CLOSE(object->get_position().x, 312.0f, epsilon);
    BOOST_CHECK_CLOSE(object->get_position().y, 112.0f, epsilon);
    BOOST_CHECK_CLOSE(object->get_size().x, 63.0f, epsilon);
    BOOST_CHECK_CLOSE(object->get_size().y, 122.0f, epsilon);
    BOOST_CHECK_EQUAL(object->get_gid(), 5u);
    BOOST_CHECK_EQUAL(object->is_visible(), false);
    BOOST_CHECK_EQUAL(object->get_sprite_filename(), "data/sprite.spr");
    BOOST_CHECK_EQUAL(static_cast<int>(object->get_direction()),
        static_cast<int>(diagonal_to_four_directions(Direction::UP | Direction::LEFT)));
    BOOST_CHECK_EQUAL(object->get_pose_name(), "POSE NAME");
    BOOST_CHECK_EQUAL(object->get_state(), "STATE NAME");
    BOOST_CHECK_EQUAL(object->get_face_state(), "FACE STATE NAME");
    BOOST_CHECK_EQUAL(object->get_walk_state(), "WALK STATE NAME");
    BOOST_CHECK_CLOSE(object->get_speed(), 4.5f, epsilon);
    BOOST_CHECK_CLOSE(object->get_movement_speed(), 4.5f, epsilon);
    BOOST_CHECK_CLOSE(object->get_animation_speed(), 3.5f, epsilon);
    BOOST_CHECK_EQUAL(color_to_hex(object->get_color()),
        color_to_hex(string_to_color("red")));
    BOOST_CHECK_CLOSE(object->get_opacity(), 0.5, epsilon);
    BOOST_CHECK_EQUAL(static_cast<int>(object->get_script_context()),
        static_cast<int>(Map_Object::Script_Context::GLOBAL));
    BOOST_CHECK_EQUAL(object->has_trigger_script(), true);
    BOOST_CHECK_EQUAL(object->get_trigger_script(), "trigger script");
    BOOST_CHECK_EQUAL(object->has_leave_script(), true);
    BOOST_CHECK_EQUAL(object->get_leave_script(), "leave script");
    BOOST_CHECK_EQUAL(object->has_touch_script(), true);
    BOOST_CHECK_EQUAL(object->get_touch_script(), "touch script");
    BOOST_CHECK_EQUAL(object->is_passthrough(), true);
    BOOST_CHECK_EQUAL(object->initiates_passthrough(), true);
    BOOST_CHECK_EQUAL(object->receives_passthrough(), false);
    BOOST_CHECK_EQUAL(static_cast<int>(object->get_passthrough_type()),
        static_cast<int>(Map_Object::Passthrough_Type::INITIATOR));
    BOOST_CHECK_EQUAL(object->get_collision_priority(), 33);
    BOOST_CHECK_EQUAL(object->get_proximity_distance(), 22);
    BOOST_CHECK_EQUAL(object->uses_layer_color(), false);
    auto outline_conditions = Map_Object::Outline_Condition::SOLID
        | Map_Object::Outline_Condition::SCRIPT
        | Map_Object::Outline_Condition::TOUCHED
        | Map_Object::Outline_Condition::PROXIMATE;
    BOOST_CHECK_EQUAL(static_cast<int>(object->get_outline_conditions()),
        static_cast<int>(outline_conditions));
    BOOST_CHECK_EQUAL(object->get_outlined_object_id(), 8);
    auto outline_color = object->get_outline_color();
    BOOST_CHECK_EQUAL(outline_color.has_value(), true);
    BOOST_CHECK_EQUAL(color_to_hex(outline_color.value()), "115511");
    BOOST_CHECK_EQUAL(static_cast<int>(object->get_draw_order()),
        static_cast<int>(Map_Object::Draw_Order::ABOVE));
    BOOST_CHECK_EQUAL(object->is_sound_attenuation_enabled(), true);
}

BOOST_AUTO_TEST_SUITE_END()
