#include "map_object_mapper.hpp"
#include "qtpropertybrowser/src/qttreepropertybrowser.h"
#include "../qtpropertybrowser/src/qtpropertymanager.h"
#include "../qtpropertybrowser/src/qtvariantproperty.h"
#include "../../include/map_object.hpp"
#include "../../include/sprite.hpp"
#include "../../include/game.hpp"
#include "../../include/utility/direction.hpp"
#include "../../include/utility/string.hpp"

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
    item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), "Direction");
    QStringList& directions = detail::get_directions();
    item->setAttribute("enumNames", directions);
    auto dir = obj->get_direction();
    if ((dir & Direction::UP) != Direction::NONE) {
        item->setValue(0);
    } else if ((dir & Direction::RIGHT) != Direction::NONE) {
        item->setValue(1);
    } else if ((dir & Direction::LEFT) != Direction::NONE) {
        item->setValue(3);
    } else {
        item->setValue(2);
    }
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
    // Player-facing
    item = manager->addProperty(QVariant::Bool, "Player-facing");
    item->setValue(obj->is_player_facing());
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
        auto value = prop_value.toString().toStdString();
        obj->set_sprite(game, value);
        obj->set_editor_property("sprite", value);
    } else if (prop_name == "Pose") {
        auto value = prop_value.toString().toStdString();
        obj->set_pose(value);
        obj->set_editor_property("pose", value);
    } else if (prop_name == "State") {
        auto value = prop_value.toString().toStdString();
        obj->set_pose("", value);
        obj->set_editor_property("state", value);
    } else if (prop_name == "Direction") {
        auto value = prop_value.toInt();
        auto dir = Direction::DOWN;
        if (value == 0) {
            dir = Direction::UP;
        } else if (value == 1) {
            dir = Direction::RIGHT;
        } else if (value == 3) {
            dir = Direction::LEFT;
        }
        obj->set_direction(dir);
        obj->set_editor_property("direction", direction_to_string(dir), "DOWN");
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
        auto value = prop_value.toFloat();
        obj->set_speed(value);
        obj->set_editor_property("speed", std::to_string(value), std::to_string(1.0f));
    } else if (prop_name == "Opacity") {
        auto value = prop_value.toFloat();
        obj->set_opacity(value);
        obj->set_editor_property("opacity", std::to_string(value), std::to_string(1.0f));
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
    } else if (prop_name == "Player-facing") {
        auto value = prop_value.toBool();
        obj->set_player_facing(value);
        obj->set_editor_property("player-facing", value ? "TRUE" : "FALSE", "TRUE");
    } else if (prop_name == "Pass-through") {
        auto value = prop_value.toBool();
        obj->set_passthrough(value);
        obj->set_editor_property("passthrough", value ? "TRUE" : "FALSE", "FALSE");
    } else if (prop_name == "Pass-through Type") {
        auto value = static_cast<Map_Object::Passthrough_Type>(prop_value.toInt() + 1);
        obj->set_passthrough_type(value);
        auto value_str = value == Map_Object::Passthrough_Type::INITIATOR ? "INITIATOR"
            : (value == Map_Object::Passthrough_Type::RECEIVER ? "RECEIVER" : "BOTH");
        obj->set_editor_property("passthrough-type", value_str, "BOTH");
    } else if (prop_name == "Draw Order") {
        auto value = static_cast<Map_Object::Draw_Order>(prop_value.toInt());
        obj->set_draw_order(value);
        auto value_str = value == Map_Object::Draw_Order::BELOW ? "BELOW"
            : (value == Map_Object::Draw_Order::ABOVE ? "ABOVE" : "NORMAL");
        obj->set_editor_property("draw-order", value_str, "NORMAL");
    } else if (prop_name == "Trigger Script") {
        auto value = prop_value.toString().toStdString();
        obj->set_trigger_script(value);
        auto script_prop = obj->get_property("script") != "" ? "script" : "trigger-script";
        obj->set_editor_property(script_prop, value, "", false);
    } else if (prop_name == "Leave Script") {
        auto value = prop_value.toString().toStdString();
        obj->set_leave_script(value);
        obj->set_editor_property("leave-script", value, "", false);
    } else if (prop_name == "Touch Script") {
        auto value = prop_value.toString().toStdString();
        obj->set_touch_script(value);
        obj->set_editor_property("touch-script", value, "", false);
    } else if (prop_name == "Script Context") {
        auto value = static_cast<Map_Object::Script_Context>(prop_value.toInt());
        obj->set_script_context(value);
        auto str_value = value == Map_Object::Script_Context::MAP ? "MAP" : "GLOBAL";
        obj->set_editor_property("script-context", str_value, "MAP");
    } else if (prop_name == "Face State") {
        auto value = prop_value.toString().toStdString();
        obj->set_face_state(value);
        obj->set_editor_property("face-state", value, "FACE");
    } else if (prop_name == "Walk State") {
        auto value = prop_value.toString().toStdString();
        obj->set_walk_state(value);
        obj->set_editor_property("walk-state", value, "WALK");
    } else if (prop_name == "Overrides Tile Collision") {
        auto value = prop_value.toBool();
        obj->set_override_tile_collision(value);
        obj->set_editor_property("override-tile-collision", value ? "TRUE" : "FALSE", "FALSE");
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
        obj->set_editor_property("outlined", condition_str.toStdString(), "TOUCHED, SCRIPT, SOLID");
    } else if (prop_name == "Outlined Object") {
        auto value = prop_value.toInt();
        obj->set_outlined_object_id(value);
        obj->set_editor_property("outlined-object", std::to_string(value), std::to_string(-1));
    }
}
