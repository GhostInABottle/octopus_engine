#include "game_fixture.hpp"
#include "../map/map.hpp"
#include "../map/layers/tile_layer.hpp"
#include "../vendor/rapidxml.hpp"
#include <boost/test/unit_test.hpp>


namespace detail {
    static void check_map(Map& map) {
        BOOST_CHECK_EQUAL(map.get_width(), 50);
        BOOST_CHECK_EQUAL(map.get_height(), 40);
        BOOST_CHECK_EQUAL(map.get_tile_width(), 8);
        BOOST_CHECK_EQUAL(map.get_tile_height(), 8);
        BOOST_CHECK_EQUAL(map.get_starting_position().x, 5);
        BOOST_CHECK_EQUAL(map.get_starting_position().y, 160);
        auto& tileset = map.get_tileset(0);
        BOOST_CHECK_EQUAL(tileset.first_id, 1);
        BOOST_CHECK_EQUAL(tileset.name, "test sheet");
        Tile_Layer* layer = static_cast<Tile_Layer*>(map.get_layer_by_index(1));
        BOOST_CHECK_EQUAL(layer->get_name(), "ground");
        BOOST_CHECK_EQUAL(layer->properties["@Description"], "ground layer");
        auto& tiles = layer->get_tiles();
        BOOST_CHECK_EQUAL(tiles[10], 132u);
    }
}

BOOST_FIXTURE_TEST_SUITE(map_tests, Game_Fixture)

BOOST_AUTO_TEST_CASE(map_load) {
    char text[] =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
        <map version=\"1.0\" orientation=\"orthogonal\" width=\"50\" height=\"40\" tilewidth=\"8\" tileheight=\"8\"> \
            <properties> \
                <property name=\"name\" value=\"testmap\"/> \
                <property name=\"player-position-x\" value=\"5\"/> \
            </properties> \
            <tileset firstgid=\"1\" name=\"test sheet\" tilewidth=\"8\" tileheight=\"8\"> \
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
            </tileset> \
            <tileset firstgid=\"513\" name=\"collision\" tilewidth=\"8\" tileheight=\"8\"> \
                <image source=\"../data/obstruction.png\" trans=\"ff00ff\" width=\"64\" height=\"8\"/> \
            </tileset> \
            <layer id=\"1\" name=\"ground\" width=\"40\" height=\"40\"> \
                <properties> \
                    <property name=\"@Description\" value=\"ground layer\"/> \
                </properties> \
                <data encoding=\"base64\" compression=\"zlib\"> \
                    eJy9mM1LVVEQwMePCsqCrAzKivAL+qCFT1tGQos+CTSotI+ViyAqkKLAwMxsayrR0q/KdhVmtouyfaVS/00zvBnuOJ5z7jnPR4sf59373r33d2fmnDPaBABNyF5DPfIUecLQ50F1blCNdG6P4x7lgNxakUbjSX5TzBgzyoypUX7j8tuKHE5km8exgR1dfjG4/OgeAxzjPCgHQ4E8kF9BxZHuPcmsx2+EY/2CGTXH+tx4wK+J3Qqc7xPIxwRCfvIO08wUv/e0+m4ywq+V/YgFpCKBGD+p12l1PApx8RNHiaPPrxbZkeD3PiEHczl+Oo7HHA47kV0eT5/faeQMcha5wONd5DZktd6miFmnZH5siohhM7Ix4HcFuYp0sxeNPchNpJ9joR1j19F6vq7FETvtJfUaEz/yO89xJL+LkM1HcUzxa/PEULwoP49y/CR+d9jvGnKdY3dDxVE8U/3EsdnETLwK/O558bOj1CP5PeZY9if61UE2p8VVnBph9XqeFz/fSI49akzxq2DHNuWo13Fxo7wcgrV76ZFA/KQOKY7noDifb5XgJ47k0KAchQaG9tmvUNxHhaGI+HWz20Mo1oz1c/VWxD5kS4Aa9pLe5wuyjPyAbL8YZw9ic879hP3GQ+o7Jqau+pRcv0NWAn4UlwMlPGcBsppP9WuHrA4nkEX2Wimjn/QCEsejENdHUu1LbdK1tM/+hOI++guy/VT8XPMnhgVmFvmDvFR8h6y/twyxXzvHT9bx4wbxGwjci+bVN8/zyO0Vj9rtM+eLcmX7VPo8o/wk9q69RvwmwN/bPkPeIG8d382ym8Runt2W2U33Z7bXped3IpeQU0wXctnhGfJ7zX70/Od8f3IeZq955bWo3HS8ZJTPM+rZEkNaa2h9bIG1fiPq/Wyf/4mfL7kb5vEvsgTFuv4Aq3tF17E+N2eeL/NE9zyaUJ8q8+q3itUSH9N9d0NWz65ebzsjPZ/9XSfntUvl2pXjPqQXuc9jrzo+6Dmf0sOH6ISsBgXt2Geeb106PN9ZFzlOdZTfy55i57Le32ocLic5hr0GV7z0se6bNwTcJP96T9F1qPf9ekecOjwxjM2vz60aqWS/ShXDQoSfdrnnybtcb/++8J0LxU/8ZD7rHPv8rI9F10TKfPA5Vpk8t/DnvPxqHqjPrn60XNTl+FlH7eXyk3dPiVneNZJj65eXV/HV1+gcVeY8V8/ZmBhqv7zeXGPzW8ra7PK282W9/19db/3Vsk+V8ZP8l8tPv39qPVYrP3ttOfxqIat51/N9a3QM5YpfTLxk3U6p1f9Vf7bfinUs1e8fGN5g0A==  \
                </data> \
            </layer> \
            <objectgroup id=\"2\" name=\"objects\"> \
            </objectgroup> \
        </map>";

    auto doc = std::make_unique<rapidxml::xml_document<>>();
    doc->parse<0>(text);
    auto node = doc->first_node("map");
    BOOST_CHECK(node);
    auto map = Map::load(*game, *node);
    detail::check_map(*map);
}

BOOST_AUTO_TEST_CASE(map_load_file) {
    auto map = Map::load(*game, "test_tiled.tmx");
    BOOST_CHECK_EQUAL(map->get_filename(), "test_tiled.tmx");
    BOOST_CHECK_EQUAL(map->get_filename_stem(), "test_tiled");
    detail::check_map(*map);
}

BOOST_AUTO_TEST_CASE(map_load_file_external_tileset) {
    auto map = Map::load(*game, "test_tiled_external_tileset.tmx");
    detail::check_map(*map);
}

BOOST_AUTO_TEST_SUITE_END()
