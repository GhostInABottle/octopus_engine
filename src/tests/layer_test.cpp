#include "game_fixture.hpp"
#include "../utility/direction.hpp"
#include "../map/layers/image_layer.hpp"
#include "../map/layers/object_layer.hpp"
#include "../map/layers/tile_layer.hpp"
#include "../map/map.hpp"
#include "../map/map_object.hpp"
#include "../vendor/rapidxml.hpp"
#include "../xd/asset_manager.hpp"
#include "../xd/audio/music.hpp"
#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(layer_tests, Game_Fixture)

BOOST_AUTO_TEST_CASE(tile_layer_load) {
    char text[] =
        "<layer id =\"1\" name=\"ground\" width=\"40\" height=\"40\"> \
            <properties> \
                <property name=\"@Description\" value=\"ground layer\"/> \
            </properties> \
            <data encoding=\"base64\" compression=\"zlib\"> \
                eJy9mM1LVVEQwMePCsqCrAzKivAL+qCFT1tGQos+CTSotI+ViyAqkKLAwMxsayrR0q/KdhVmtouyfaVS/00zvBnuOJ5z7jnPR4sf59373r33d2fmnDPaBABNyF5DPfIUecLQ50F1blCNdG6P4x7lgNxakUbjSX5TzBgzyoypUX7j8tuKHE5km8exgR1dfjG4/OgeAxzjPCgHQ4E8kF9BxZHuPcmsx2+EY/2CGTXH+tx4wK+J3Qqc7xPIxwRCfvIO08wUv/e0+m4ywq+V/YgFpCKBGD+p12l1PApx8RNHiaPPrxbZkeD3PiEHczl+Oo7HHA47kV0eT5/faeQMcha5wONd5DZktd6miFmnZH5siohhM7Ix4HcFuYp0sxeNPchNpJ9joR1j19F6vq7FETvtJfUaEz/yO89xJL+LkM1HcUzxa/PEULwoP49y/CR+d9jvGnKdY3dDxVE8U/3EsdnETLwK/O558bOj1CP5PeZY9if61UE2p8VVnBph9XqeFz/fSI49akzxq2DHNuWo13Fxo7wcgrV76ZFA/KQOKY7noDifb5XgJ47k0KAchQaG9tmvUNxHhaGI+HWz20Mo1oz1c/VWxD5kS4Aa9pLe5wuyjPyAbL8YZw9ic879hP3GQ+o7Jqau+pRcv0NWAn4UlwMlPGcBsppP9WuHrA4nkEX2Wimjn/QCEsejENdHUu1LbdK1tM/+hOI++guy/VT8XPMnhgVmFvmDvFR8h6y/twyxXzvHT9bx4wbxGwjci+bVN8/zyO0Vj9rtM+eLcmX7VPo8o/wk9q69RvwmwN/bPkPeIG8d382ym8Runt2W2U33Z7bXped3IpeQU0wXctnhGfJ7zX70/Od8f3IeZq955bWo3HS8ZJTPM+rZEkNaa2h9bIG1fiPq/Wyf/4mfL7kb5vEvsgTFuv4Aq3tF17E+N2eeL/NE9zyaUJ8q8+q3itUSH9N9d0NWz65ebzsjPZ/9XSfntUvl2pXjPqQXuc9jrzo+6Dmf0sOH6ISsBgXt2Geeb106PN9ZFzlOdZTfy55i57Le32ocLic5hr0GV7z0se6bNwTcJP96T9F1qPf9ekecOjwxjM2vz60aqWS/ShXDQoSfdrnnybtcb/++8J0LxU/8ZD7rHPv8rI9F10TKfPA5Vpk8t/DnvPxqHqjPrn60XNTl+FlH7eXyk3dPiVneNZJj65eXV/HV1+gcVeY8V8/ZmBhqv7zeXGPzW8ra7PK282W9/19db/3Vsk+V8ZP8l8tPv39qPVYrP3ttOfxqIat51/N9a3QM5YpfTLxk3U6p1f9Vf7bfinUs1e8fGN5g0A==  \
            </data> \
        </layer>";
    auto doc = std::make_unique<rapidxml::xml_document<>>();
    doc->parse<0>(text);
    auto node = doc->first_node("layer");
    BOOST_CHECK(node);
    auto layer = Tile_Layer::load(*node, *game->get_camera());
    BOOST_CHECK_EQUAL(layer->get_name(), "ground");
    BOOST_CHECK_EQUAL(layer->get_width(), 40);
    BOOST_CHECK_EQUAL(layer->get_height(), 40);
    BOOST_CHECK_EQUAL(layer->get_opacity(), 1.0f);
    BOOST_CHECK_EQUAL(layer->is_visible(), true);
    BOOST_CHECK_EQUAL(layer->properties["@Description"], "ground layer");
    auto tile_layer = static_cast<Tile_Layer*>(layer.get());
    auto& tiles = tile_layer->get_tiles();
    BOOST_CHECK_EQUAL(tiles[0], 37u);
    BOOST_CHECK_EQUAL(tiles[10], 132u);
}

BOOST_AUTO_TEST_CASE(image_layer_load) {
    char text[] =
        "<imagelayer id=\"1\" name=\"some image\" width=\"40\" height=\"40\" opacity=\"0.55\" visible=\"0\" > \
            <image source=\"../data/test_tileset.gif\" trans=\"ff5fff\"/> \
            <properties> \
                <property name=\"test\" value=\"1\"/> \
            </properties> \
        </imagelayerlayer>";
    auto doc = std::make_unique<rapidxml::xml_document<>>();
    doc->parse<0>(text);
    auto node = doc->first_node("imagelayer");
    BOOST_CHECK(node);
    xd::asset_manager manager;
    auto layer = Image_Layer::load(*node, *game, *game->get_camera(), manager);
    BOOST_CHECK_EQUAL(layer->get_name(), "some image");
    BOOST_CHECK_CLOSE(layer->get_opacity(), 0.55f, 0.001f);
    BOOST_CHECK_EQUAL(layer->is_visible(), false);
    BOOST_CHECK_EQUAL(layer->properties["test"], "1");
   auto image_layer = static_cast<Image_Layer*>(layer.get());
    BOOST_CHECK_EQUAL(image_layer->get_image_filename(), "../data/test_tileset.gif");
    BOOST_CHECK_CLOSE(image_layer->get_transparent_color().g, 0.372549f, 0.1f);
    BOOST_CHECK(image_layer->get_texture());
}

BOOST_AUTO_TEST_CASE(object_layer_load) {
    char text[] =
        "<objectgroup  id=\"1\" color=\"#ff0000\" name=\"obj layer\" width=\"40\" height=\"40\"> \
            <properties> \
                <property name=\"jimbo\" value=\"yeah right\"/> \
            </properties> \
            <object id =\"5\" name=\"cool\" type=\"dragon\" x=\"168\" y=\"208\"> \
                <properties> \
                    <property name=\"elf\" value=\"qui\"/> \
                    <property name=\"direction\" value=\"right\"/> \
                    <property name=\"pose\" value=\"mypose\"/> \
                    <property name=\"state\" value=\"mystate\"/> \
                    <property name=\"speed\" value=\"3.5\"/> \
                    <property name=\"opacity\" value=\"0.4\"/> \
                    <property name=\"face-state\" value=\"facing\"/> \
                    <property name=\"walk-state\" value=\"walking\"/> \
                </properties> \
            </object> \
            <object id =\"6\" name=\"collision\" x=\"146\" y=\"0\" width=\"8\" height=\"16\"/> \
        </objectgroup>";
    auto doc = std::make_unique<rapidxml::xml_document<>>();
    doc->parse<0>(text);
    auto node = doc->first_node("objectgroup");
    Map map(*game);
    auto layer = Object_Layer::load(*node, *game, *game->get_camera(), map);
    BOOST_CHECK_EQUAL(layer->get_name(), "obj layer");
    BOOST_CHECK_EQUAL(layer->get_width(), 40);
    BOOST_CHECK_EQUAL(layer->get_height(), 40);
    BOOST_CHECK_CLOSE(layer->get_opacity(), 1.0f, 0.001f);
    BOOST_CHECK_EQUAL(layer->is_visible(), true);
    BOOST_CHECK_EQUAL(layer->properties["jimbo"], "yeah right");
    auto object_layer = static_cast<Object_Layer*>(layer.get());
    auto& objects = object_layer->get_objects();
    auto& obj1 = *objects[0];
    auto& obj2 = *objects[1];
    BOOST_CHECK_EQUAL(obj1.get_id(), 5);
    BOOST_CHECK_EQUAL(obj1.get_name(), "COOL");
    BOOST_CHECK_EQUAL(obj1.get_type(), "dragon");
    BOOST_CHECK_CLOSE(obj1.get_position().x, 168.0f, 0.1f);
    BOOST_CHECK_CLOSE(obj1.get_position().y, 208.0f, 0.1f);
    BOOST_CHECK_CLOSE(obj1.get_size()[0], 0.0f, 0.1f);
    BOOST_CHECK_CLOSE(obj1.get_size()[1], 0.0f, 0.1f);
    BOOST_CHECK_EQUAL(obj1.get_property("elf"), "qui");
    BOOST_CHECK_EQUAL(obj1.get_direction(), Direction::RIGHT);
    BOOST_CHECK_EQUAL(obj1.get_pose_name(), "MYPOSE");
    BOOST_CHECK_EQUAL(obj1.get_state(), "MYSTATE");
    BOOST_CHECK_CLOSE(obj1.get_speed(), 3.5f, 0.1f);
    BOOST_CHECK_CLOSE(obj1.get_opacity(), 0.4f, 0.1f);
    BOOST_CHECK_EQUAL(obj1.get_face_state(), "FACING");
    BOOST_CHECK_EQUAL(obj1.get_walk_state(), "WALKING");
    BOOST_CHECK_EQUAL(obj2.get_id(), 6);
    BOOST_CHECK_EQUAL(obj2.get_name(), "COLLISION");
    BOOST_CHECK_CLOSE(obj2.get_size()[0], 8.0f, 0.1f);
    BOOST_CHECK_CLOSE(obj2.get_size()[1], 16.0f, 0.1f);
}

BOOST_AUTO_TEST_SUITE_END()
