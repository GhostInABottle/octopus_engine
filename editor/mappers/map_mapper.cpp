#include "map_mapper.hpp"
#include "qtpropertybrowser/src/qttreepropertybrowser.h"
#include "../qtpropertybrowser/src/qtpropertymanager.h"
#include "../qtpropertybrowser/src/qtvariantproperty.h"
#include "../../src/map/map.hpp"

Map_Mapper::Map_Mapper(Map* map) : map(map) {}

void Map_Mapper::populate(QtTreePropertyBrowser* browser, QtVariantPropertyManager* manager) {
    if (!map)
        return;
    manager->clear();
    // Name
    QtVariantProperty* item = manager->addProperty(QVariant::String, "Name");
    item->setValue(QString::fromStdString(map->get_name()));
    browser->addProperty(item);
    // Size
    item = manager->addProperty(QVariant::Size, "Size");
    item->setValue(QSize(map->get_width(), map->get_height()));
    browser->addProperty(item);
    // Music
    item = manager->addProperty(QVariant::String, "Music");
    item->setValue(QString::fromStdString(map->get_bg_music_filename()));
    browser->addProperty(item);
    // Music Script
    item = manager->addProperty(QVariant::String, "Music Script");
    item->setValue(QString::fromStdString(map->get_property("music-script")));
    browser->addProperty(item);
    // Scripts
    item = manager->addProperty(QVariant::String, "Scripts");
    item->setValue(QString::fromStdString(map->get_startup_scripts()));
    browser->addProperty(item);
    // Player Position
    item = manager->addProperty(QVariant::Point, "Player Position");
    auto pos = map->get_starting_position();
    item->setValue(QPoint(static_cast<int>(pos.x), static_cast<int>(pos.y)));
    browser->addProperty(item);
}

void Map_Mapper::change_property(QtProperty* prop) {
    if (!map)
        return;
    QtVariantProperty* vprop = static_cast<QtVariantProperty*>(prop);
    QString prop_name = prop->propertyName();
    QVariant prop_value = vprop->value();
    if (prop_name == "Name") {
        map->set_name(prop_value.toString().toStdString());
    } else if (prop_name == "Size") {
        QSize sz = prop_value.toSize();
        xd::ivec2 map_size(sz.width(), sz.height());
        xd::ivec2 tile_size(map->get_tile_width(), map->get_tile_height());
        map->resize(map_size, tile_size);
    } else if (prop_name == "Music") {
        map->set_bg_music_filename(prop_value.toString().toStdString());
    } else if (prop_name == "Music Script") {
        auto value = prop_value.toString().toStdString();
        map->set_editor_property("music-script", value, "", false);
    } else if (prop_name == "Scripts") {
        map->set_startup_scripts(prop_value.toString().toStdString());
    } else if (prop_name == "Player Position") {
        auto value = prop_value.toPoint();
        map->set_starting_position(xd::vec2{value.x(), value.y()});
        map->set_editor_property("player-position-x", std::to_string(value.x()));
        map->set_editor_property("player-position-y", std::to_string(value.y()));
    }
}
