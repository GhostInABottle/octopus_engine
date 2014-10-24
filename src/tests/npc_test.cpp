#include <boost/test/unit_test.hpp>
#include "../../include/rapidxml.hpp"
#include "../../include/npc.hpp"
#include "../../include/keypoint.hpp"
#include <iostream>

namespace detail {
    extern Game* game;
}

BOOST_AUTO_TEST_CASE(npc_load) {
    char text[] = 
        "<npc name=\"Stinkdad\" sprite=\"sprite.spr\"> \
            <properties> \
                <property name=\"a\" value=\"b\" /> \
            </properties> \
            <schedule name=\"first\"> \
                <keypoint map=\"room\" pose=\"main\" x=\"1\" y=\"2\" activation=\"script.lua\"> \
                    <time day=\"2\" timestamp=\"6:11:40\" /> \
                    <commands> \
                        <command type=\"move\" x=\"30\" y=\"40\" /> \
                        <command type=\"wait\" duration=\"10\" /> \
                        <command type=\"teleport\" map=\"doom\" x=\"1\" y=\"2\" /> \
                        <command type=\"visibility\" value=\"false\" /> \
                        <command type=\"passthrough\" value=\"true\" /> \
                        <command type=\"wait\" duration=\"5:12:40\" /> \
                    </commands> \
                </keypoint> \
                <keypoint map=\"doom\" x=\"3\" y=\"4\" sequential=\"true\"> \
                    <time day=\"5\" timestamp=\"30311\" /> \
                </keypoint> \
            </scedule> \
            <schedule name=\"second\">  \
                <keypoint map=\"boom\" x=\"5\" y=\"6\"> \
                    <time timestamp=\"11:55\" /> \
                </keypoint> \
            </scedule> \
        </npc>";
    rapidxml::xml_document<> doc;
    doc.parse<0>(text);
    rapidxml::xml_node<>* node = doc.first_node("npc");
    BOOST_CHECK(node);
    auto npc = NPC::load(*detail::game, *node);
    // Test NPC fields
    BOOST_CHECK_EQUAL(npc->get_name(), "Stinkdad");
    // Test first keypoint in first schedule
    BOOST_CHECK(npc->get_keypoint("first", 0));
    auto& keypoint_1a = *npc->get_keypoint("first", 0);
    BOOST_CHECK_EQUAL(keypoint_1a.map, "room");
    BOOST_CHECK_EQUAL(keypoint_1a.pose, "main");
    BOOST_CHECK_EQUAL(keypoint_1a.position.x, 1.0f);
    BOOST_CHECK_EQUAL(keypoint_1a.position.y, 2.0f);
    BOOST_CHECK(!keypoint_1a.sequential);
    BOOST_CHECK_EQUAL(keypoint_1a.activation_script, "script.lua");
    BOOST_CHECK_EQUAL(keypoint_1a.day, 2);
    BOOST_CHECK_EQUAL(keypoint_1a.hour(), 6);
    BOOST_CHECK_EQUAL(keypoint_1a.minute(), 11);
    BOOST_CHECK_EQUAL(keypoint_1a.second(), 40);
    BOOST_CHECK_EQUAL(keypoint_1a.commands.size(), 6);
    // Test keypoint commands
    auto& cmds = keypoint_1a.commands;
    BOOST_CHECK(cmds[0].type == Keypoint::Command::Types::MOVE);
    BOOST_CHECK_EQUAL(boost::any_cast<float>(cmds[0].args["x"]), 30.0f);
    BOOST_CHECK_EQUAL(boost::any_cast<float>(cmds[0].args["y"]), 40.0f);
    BOOST_CHECK(cmds[1].type == Keypoint::Command::Types::WAIT);
    BOOST_CHECK_EQUAL(boost::any_cast<int>(cmds[1].args["duration"]), 10);
    BOOST_CHECK(cmds[2].type == Keypoint::Command::Types::TELEPORT);
    BOOST_CHECK_EQUAL(boost::any_cast<std::string>(cmds[2].args["map"]), "doom");
    BOOST_CHECK_EQUAL(boost::any_cast<float>(cmds[2].args["x"]), 1.0f);
    BOOST_CHECK_EQUAL(boost::any_cast<float>(cmds[2].args["y"]), 2.0f);
    BOOST_CHECK(cmds[3].type == Keypoint::Command::Types::VISIBILITY);
    BOOST_CHECK(!boost::any_cast<bool>(cmds[3].args["value"]));
    BOOST_CHECK(cmds[4].type == Keypoint::Command::Types::PASSTHROUGH);
    BOOST_CHECK(boost::any_cast<bool>(cmds[4].args["value"]));
    BOOST_CHECK_EQUAL(boost::any_cast<int>(cmds[5].args["duration"]), 18760);
    // Test second keypoint
    BOOST_CHECK(npc->get_keypoint("first", 1));
    auto& keypoint_2a = *npc->get_keypoint("first", 1);
    BOOST_CHECK_EQUAL(keypoint_2a.map, "doom");
    BOOST_CHECK_EQUAL(keypoint_2a.position.x, 3.0f);
    BOOST_CHECK_EQUAL(keypoint_2a.position.y, 4.0f);
    BOOST_CHECK(keypoint_2a.sequential);
    BOOST_CHECK_EQUAL(keypoint_2a.day, 5);
    BOOST_CHECK_EQUAL(keypoint_2a.hour(), 8);
    BOOST_CHECK_EQUAL(keypoint_2a.minute(), 25);
    BOOST_CHECK_EQUAL(keypoint_2a.second(), 11);
    BOOST_CHECK_EQUAL(keypoint_2a.commands.size(), 0);
    // Test keypoint in second schedule
    BOOST_CHECK(npc->get_keypoint("second", 0));
    auto& keypoint_b = *npc->get_keypoint("second", 0);
    BOOST_CHECK_EQUAL(keypoint_b.map, "boom");
    BOOST_CHECK_EQUAL(keypoint_b.position.x, 5.0f);
    BOOST_CHECK_EQUAL(keypoint_b.position.y, 6.0f);
    BOOST_CHECK_EQUAL(keypoint_b.day, 1);
    BOOST_CHECK_EQUAL(keypoint_b.hour(), 11);
    BOOST_CHECK_EQUAL(keypoint_b.minute(), 55);
    BOOST_CHECK_EQUAL(keypoint_b.second(), 0);
}
