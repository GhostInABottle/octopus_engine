#include "map_object_mapper.hpp"
#include "qtpropertybrowser/src/qttreepropertybrowser.h"
#include "../qtpropertybrowser/src/qtpropertymanager.h"
#include "../qtpropertybrowser/src/qtvariantproperty.h"
#include "../../include/map_object.hpp"
#include "../../include/sprite.hpp"
#include "../../include/game.hpp"

namespace detail {
    QStringList types;
    QStringList& get_types() {
        if (types.empty())
            types << "npc" << "prop" << "area" << "area object";
        return types;
    }
    void add_type(QString type) {
        types << type;
    }
    QStringList& get_directions() {
        static QStringList directions;
        if (directions.empty())
            directions << "UP" << "RIGHT" << "DOWN" << "LEFT";
        return directions;
    }
    QStringList& get_draw_orders() {
        static QStringList orders;
        if (orders.empty())
            orders << "BELOW" << "NORMAL" << "ABOVE";
        return orders;
    }
}

Map_Object_Mapper::Map_Object_Mapper(Game& game, Map_Object* object) : game(game), obj(object) {}

void Map_Object_Mapper::populate(QtTreePropertyBrowser* browser, QtVariantPropertyManager* manager) {
    if (!obj)
        return;
    manager->clear();
    // Name
    QtVariantProperty* item = manager->addProperty(QVariant::String, "Name");
    item->setValue(QString::fromStdString(obj->get_name()));
    browser->addProperty(item);
    // Type
    item = manager->addProperty(QtVariantPropertyManager::enumTypeId(),
                    "Type");
    QStringList& types = detail::get_types();
    QString type = QString::fromStdString(obj->get_type());
    type = type.toLower();
    int index = types.indexOf(type);
    if (index == -1) {
        detail::add_type(type);
        index = types.size() - 1;
    }
    item->setAttribute("enumNames", types);
    item->setValue(index);
    browser->addProperty(item);
    // Sprite
    item = manager->addProperty(QVariant::String, "Sprite");
    if (auto sprite = obj->get_sprite())
        item->setValue(QString::fromStdString(sprite->get_filename()));
    browser->addProperty(item);
    // Pose
    item = manager->addProperty(QVariant::String, "Pose");
    item->setValue(QString::fromStdString(obj->get_pose_name()));
    browser->addProperty(item);
    // State
    item = manager->addProperty(QVariant::String, "State");
    item->setValue(QString::fromStdString(obj->get_state()));
    browser->addProperty(item);
    // Direction
    item = manager->addProperty(QtVariantPropertyManager::flagTypeId(),
                    "Direction");
    QStringList& directions = detail::get_directions();
    item->setAttribute("flagNames", directions);
    item->setValue(static_cast<int>(obj->get_direction()));
    browser->addProperty(item);
    // Position
    item = manager->addProperty(QVariant::Point, "Position");
    int x = static_cast<int>(obj->get_x());
    int y = static_cast<int>(obj->get_y());
    item->setValue(QPoint(x, y));
    browser->addProperty(item);
    // Size
    item = manager->addProperty(QVariant::Size, "Size");
    int w = static_cast<int>(obj->get_size().x);
    int h = static_cast<int>(obj->get_size().y);
    item->setValue(QSize(w, h));
    browser->addProperty(item);
    // Color
    item = manager->addProperty(QVariant::Color,  "Color");
    auto c = obj->get_color();
    item->setValue(QColor::fromRgbF(c.r, c.g, c.b, c.a));
    browser->addProperty(item);
    // Speed
    item = manager->addProperty(QVariant::Double, "Speed");
    item->setValue(obj->get_speed());
    item->setAttribute("singleStep", 0.01);
    item->setAttribute("decimals", 3);
    browser->addProperty(item);
    // Opacity
    item = manager->addProperty(QVariant::Double, "Opacity");
    item->setValue(obj->get_opacity());
    item->setAttribute("singleStep", 0.01);
    item->setAttribute("decimals", 3);
    browser->addProperty(item);
    // Angle
    item = manager->addProperty(QVariant::Int, "Angle");
    item->setValue(obj->get_angle());
    browser->addProperty(item);
    // Visible
    item = manager->addProperty(QVariant::Bool, "Visible");
    item->setValue(obj->is_visible());
    browser->addProperty(item);
    // Disabled
    item = manager->addProperty(QVariant::Bool, "Disabled");
    item->setValue(obj->is_disabled());
    browser->addProperty(item);
    // Stopped
    item = manager->addProperty(QVariant::Bool, "Stopped");
    item->setValue(obj->is_stopped());
    browser->addProperty(item);
    // Frozen
    item = manager->addProperty(QVariant::Bool, "Frozen");
    item->setValue(obj->is_disabled());
    browser->addProperty(item);
    // Pass-through
    item = manager->addProperty(QVariant::Bool, "Pass-through");
    item->setValue(obj->is_passthrough());
    browser->addProperty(item);
    // Draw order
    item = manager->addProperty(QtVariantPropertyManager::enumTypeId(),
                    "Draw Order");
    QStringList& orders = detail::get_draw_orders();
    item->setAttribute("enumNames", orders);
    item->setValue(static_cast<int>(obj->get_draw_order()));
    browser->addProperty(item);
    // Trigger script
    item = manager->addProperty(QVariant::String, "Trigger Script");
    item->setValue(QString::fromStdString(obj->get_trigger_script()));
    browser->addProperty(item);
    // Leave script
    item = manager->addProperty(QVariant::String, "Leave Script");
    item->setValue(QString::fromStdString(obj->get_leave_script()));
    browser->addProperty(item);
    // Touch script
    item = manager->addProperty(QVariant::String, "Touch Script");
    item->setValue(QString::fromStdString(obj->get_touch_script()));
    browser->addProperty(item);
}

void Map_Object_Mapper::change_property(QtProperty* prop) {
    if (!obj)
        return;
    QtVariantProperty* vprop = static_cast<QtVariantProperty*>(prop);
    QString prop_name = prop->propertyName();
    QVariant prop_value = vprop->value();
    if (prop_name == "Name") {
        obj->set_name(prop_value.toString().toStdString());
    } else if (prop_name == "Type") {
        QStringList& types = detail::get_types();
        obj->set_type(types[prop_value.toInt()].toStdString());
    } else if (prop_name == "Sprite") {
        obj->set_sprite(game, prop_value.toString().toStdString());
    } else if (prop_name == "Pose") {
        obj->set_pose(prop_value.toString().toStdString());
    } else if (prop_name == "State") {
        obj->set_pose("", prop_value.toString().toStdString());
    } else if (prop_name == "Direction") {
        obj->set_direction(static_cast<Direction>(prop_value.toInt()));
    } else if (prop_name == "Position") {
        QPoint pos = prop_value.toPoint();
        obj->set_position(xd::vec2(pos.x(), pos.y()));
    } else if (prop_name == "Size") {
        QSize size = prop_value.toSize();
        obj->set_size(xd::vec2(size.width(), size.height()));
    } else if (prop_name == "Color") {
        QColor c = prop_value.value<QColor>();
        obj->set_color(xd::vec4(c.redF(), c.greenF(), c.blueF(), c.alphaF()));
    } else if (prop_name == "Speed") {
        obj->set_speed(prop_value.toFloat());
    } else if (prop_name == "Opacity") {
        obj->set_opacity(prop_value.toFloat());
    } else if (prop_name == "Angle") {
        obj->set_angle(prop_value.toInt());
    } else if (prop_name == "Visible") {
        obj->set_visible(prop_value.toBool());
    } else if (prop_name == "Disabled") {
        obj->set_disabled(prop_value.toBool());
    } else if (prop_name == "Stopped") {
        obj->set_stopped(prop_value.toBool());
    } else if (prop_name == "Frozen") {
        obj->set_frozen(prop_value.toBool());
    } else if (prop_name == "Pass-through") {
        obj->set_passthrough(prop_value.toBool());
    } else if (prop_name == "Draw Order") {
        obj->set_draw_order(static_cast<Map_Object::Draw_Order>(prop_value.toInt()));
    } else if (prop_name == "Trigger Script") {
        obj->set_trigger_script(prop_value.toString().toStdString());
    } else if (prop_name == "Leave Script") {
        obj->set_leave_script(prop_value.toString().toStdString());
    } else if (prop_name == "Touch Script") {
        obj->set_touch_script(prop_value.toString().toStdString());
    }
}
