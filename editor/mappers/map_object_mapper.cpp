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
    QStringList& get_script_contexts() {
        static QStringList contexts;
        if (contexts.empty())
            contexts << "MAP" << "GLOBAL";
        return contexts;
    }
    QStringList& get_passthrough_types() {
        static QStringList passthrough_types;
        if (passthrough_types.empty())
            passthrough_types << "INITIATOR" << "RECEIVER" << "BOTH";
        return passthrough_types;
    }
    QStringList& get_outline_conditions() {
        static QStringList outline_conditions;
        if (outline_conditions.empty())
            outline_conditions << "ALWAYS" << "NEVER" << "TOUCHED" <<
                                  "SCRIPT" << "SOLID" << "TOUCHED, SCRIPT" <<
                                  "TOUCHED, SOLID" << "SCRIPT, SOLID" <<
                                  "TOUCHED, SCRIPT, SOLID";
        return outline_conditions;
    }
}

Map_Object_Mapper::Map_Object_Mapper(Game& game, Map_Object* object) : game(game), obj(object) {}

void Map_Object_Mapper::populate(QtTreePropertyBrowser* browser, QtVariantPropertyManager* manager) {
    if (!obj)
        return;
    manager->clear();
    // ID
    QtVariantProperty* item = manager->addProperty(QVariant::Int, "Name");
    item->setEnabled(false);
    item->setValue(obj->get_id());
    browser->addProperty(item);
    // Name
    item = manager->addProperty(QVariant::String, "Name");
    item->setValue(QString::fromStdString(obj->get_name()));
    browser->addProperty(item);
    // Type
    item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), "Type");
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
    // Pass-through Type
    item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), "Pass-through Type");
    auto& passthroughTypes = detail::get_passthrough_types();
    item->setAttribute("enumNames", passthroughTypes);
    item->setValue(static_cast<int>(obj->get_passthrough_type()) - 1);
    browser->addProperty(item);
    // Draw order
    item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), "Draw Order");
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
    // Script context
    item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), "Script Context");
    QStringList& contexts = detail::get_script_contexts();
    item->setAttribute("enumNames", contexts);
    item->setValue(static_cast<int>(obj->get_script_context()));
    browser->addProperty(item);
    // Default face state
    item = manager->addProperty(QVariant::String, "Face State");
    item->setValue(QString::fromStdString(obj->get_face_state()));
    browser->addProperty(item);
    // Default walk state
    item = manager->addProperty(QVariant::String, "Walk State");
    item->setValue(QString::fromStdString(obj->get_walk_state()));
    browser->addProperty(item);
    // Overrides Tile Collision
    item = manager->addProperty(QVariant::Bool, "Overrides Tile Collision");
    item->setValue(obj->overrides_tile_collision());
    browser->addProperty(item);
    // Outlined
    item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), "Outlined");
    auto& conditions = detail::get_outline_conditions();
    item->setAttribute("enumNames", conditions);
    auto obj_conditions = obj->get_outline_conditions();
    if (obj_conditions == Map_Object::Outline_Condition::NONE) {
        item->setValue(0);
    } else if ((obj_conditions & Map_Object::Outline_Condition::NEVER) != Map_Object::Outline_Condition::NONE) {
        item->setValue(1);
    } else {
        auto touched = (obj_conditions & Map_Object::Outline_Condition::TOUCHED) != Map_Object::Outline_Condition::NONE;
        auto script = (obj_conditions & Map_Object::Outline_Condition::SCRIPT) != Map_Object::Outline_Condition::NONE;
        auto solid = (obj_conditions & Map_Object::Outline_Condition::SOLID) != Map_Object::Outline_Condition::NONE;
        QString condition_str;
        if (touched) {
            condition_str += "TOUCHED";
        }
        if (script) {
            if (!condition_str.isEmpty()) {
                condition_str += ", ";
            }
            condition_str += "SCRIPT";
        }
        if (solid) {
            if (!condition_str.isEmpty()) {
                condition_str += ", ";
            }
            condition_str += "SOLID";
        }
        auto index = conditions.indexOf(condition_str);
        item->setValue(index < 0 ? 0 : index);
    }
    browser->addProperty(item);
    // Outlined Object
    item = manager->addProperty(QVariant::Int, "Outlined Object");
    item->setValue(obj->get_outlined_object_id());
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
    } else if (prop_name == "Pass-through Type") {
        obj->set_passthrough_type(static_cast<Map_Object::Passthrough_Type>(prop_value.toInt() + 1));
    } else if (prop_name == "Draw Order") {
        obj->set_draw_order(static_cast<Map_Object::Draw_Order>(prop_value.toInt()));
    } else if (prop_name == "Trigger Script") {
        obj->set_trigger_script(prop_value.toString().toStdString());
    } else if (prop_name == "Leave Script") {
        obj->set_leave_script(prop_value.toString().toStdString());
    } else if (prop_name == "Touch Script") {
        obj->set_touch_script(prop_value.toString().toStdString());
    } else if (prop_name == "Script Context") {
        obj->set_script_context(static_cast<Map_Object::Script_Context>(prop_value.toInt()));
    } else if (prop_name == "Face State") {
        obj->set_face_state(prop_value.toString().toStdString());
    } else if (prop_name == "Walk State") {
        obj->set_walk_state(prop_value.toString().toStdString());
    } else if (prop_name == "Overrides Tile Collision") {
        obj->set_override_tile_collision(prop_value.toBool());
    } else if (prop_name == "Outlined") {
        auto& conditions = detail::get_outline_conditions();
        auto condition_str = conditions[prop_value.toInt()];
        if (condition_str == "ALWAYS") {
            obj->set_outline_conditions(Map_Object::Outline_Condition::NONE);
        } else if (condition_str == "NEVER") {
            obj->set_outline_conditions(Map_Object::Outline_Condition::NEVER);
        } else {
            auto new_condition = Map_Object::Outline_Condition::NONE;
            if (condition_str.contains("TOUCHED")) {
                new_condition = new_condition | Map_Object::Outline_Condition::TOUCHED;
            }
            if (condition_str.contains("SCRIPT")) {
                new_condition = new_condition | Map_Object::Outline_Condition::SCRIPT;
            }
            if (condition_str.contains("SOLID")) {
                new_condition = new_condition | Map_Object::Outline_Condition::SOLID;
            }
            obj->set_outline_conditions(new_condition);
        }
    } else if (prop_name == "Outlined Object") {
        obj->set_outlined_object_id(prop_value.toInt());
    }
}
