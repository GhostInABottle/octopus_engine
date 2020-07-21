#include <boost/test/unit_test.hpp>
#include "../../include/vendor/rapidxml.hpp"
#include "../../include/tileset.hpp"

BOOST_AUTO_TEST_SUITE(tileset_tests)

BOOST_AUTO_TEST_CASE(tileset_load) {
    char text[] =
        "<tileset firstgid=\"1\" name=\"test sheet\" tilewidth=\"8\" tileheight=\"8\"> \
            <properties> \
                <property name=\"prop1\" value=\"1\"/> \
            </properties> \
            <image source=\"../data/test_tileset.gif\" trans=\"ff00ff\" width=\"128\" height=\"256\"/> \
            <tile id=\"0\"> \
                <properties> \
                    <property name=\"a\" value=\"b\"/> \
                    <property name=\"x\" value=\"y\"/> \
                </properties> \
            </tile> \
            <tile id=\"272\"> \
                <properties> \
                    <property name=\"test\" value=\"va\"/> \
                </properties> \
            </tile> \
        </tileset>";
    auto doc = std::make_unique<rapidxml::xml_document<>>();
    doc->parse<0>(text);
    auto node = doc->first_node("tileset");
    BOOST_CHECK(node);
    auto tileset = Tileset::load(*node);
    BOOST_CHECK_EQUAL(tileset->first_id, 1);
    BOOST_CHECK_EQUAL(tileset->name, "test sheet");
    BOOST_CHECK_EQUAL(tileset->tile_width, 8);
    BOOST_CHECK_EQUAL(tileset->tile_height, 8);
    BOOST_CHECK_EQUAL(tileset->properties["prop1"], "1");
    BOOST_CHECK_EQUAL(tileset->image_source, "../data/test_tileset.gif");
    BOOST_CHECK_EQUAL(tileset->tiles.size(), 2);
    BOOST_CHECK_EQUAL(tileset->tiles[0].properties["x"], "y");
    BOOST_CHECK_EQUAL(tileset->tiles[1].id, 272);
    BOOST_CHECK_EQUAL(tileset->tiles[1].properties["test"], "va");
}

BOOST_AUTO_TEST_SUITE_END()
