#include "../include/map_object.hpp"
#include "../include/game.hpp"
#include "../include/map.hpp"
#include "../include/sprite.hpp"
#include "../include/sprite_data.hpp"
#include "../include/utility/direction.hpp"
#include "../include/utility/file.hpp"
#include "../include/utility/math.hpp"
#include "../include/utility/xml.hpp"
#include "../include/utility/string.hpp"
#include "../include/exceptions.hpp"
#include "../include/log.hpp"
#include "../include/configurations.hpp"
#include <utility>

Map_Object::Outline_Condition operator|(Map_Object::Outline_Condition a, Map_Object::Outline_Condition b)
{
    return static_cast<Map_Object::Outline_Condition>(static_cast<int>(a) | static_cast<int>(b));
}

Map_Object::Outline_Condition operator&(Map_Object::Outline_Condition a, Map_Object::Outline_Condition b)
{
    return static_cast<Map_Object::Outline_Condition>(static_cast<int>(a) & static_cast<int>(b));
}

Map_Object::Map_Object(Game& game, const std::string& name,
        std::string sprite_file, xd::vec2 pos, Direction dir) :
        game(game),
        layer(nullptr),
        id(-1),
        name(name),
        position(pos),
        color(1.0f),
        magnification(1.0f),
        outline_conditions(get_default_outline_conditions()),
        outlined_object_id(-1),
        outlining_object(nullptr),
        gid(0),
        opacity(1.0f),
        visible(true),
        disabled(false),
        stopped(false),
        frozen(false),
        passthrough(false),
        passthrough_type(Passthrough_Type::BOTH),
        override_tile_collision(false),
        direction(dir),
        state("FACE"),
        face_state("FACE"),
        walk_state("WALK"),
        script_context(Script_Context::MAP),
        player_facing(true),
        collision_object(nullptr),
        collision_area(nullptr),
        triggered_object(nullptr),
        draw_order(Draw_Order::NORMAL),
        speed(1.0f) {
    if (!sprite_file.empty()) {
        set_sprite(game, sprite_file);
    }
    const auto bounding_box = get_bounding_box();
    position.x -= bounding_box.x;
    position.y -= bounding_box.y;
    set_speed(1.0f);
}

Collision_Record Map_Object::move(Direction move_dir, float pixels,
        Collision_Check_Type check_type, bool change_facing) {
    // Map relative directions
    if (move_dir == Direction::FORWARD)
        move_dir = direction;
    else if (move_dir == Direction::BACKWARD)
        move_dir = opposite_direction(direction);

    const auto movement_vector = direction_to_vector(move_dir);
    auto change = movement_vector * pixels;
    const auto x_changed = !check_close(change.x, 0.0f);
    const auto y_changed = !check_close(change.y, 0.0f);
    const auto multiple_directions = x_changed && y_changed;
    const auto movement = x_changed || y_changed;

    if (!movement) {
        set_state(face_state);
        // If there was no movement there's no need to check tile collision,
        // but maybe we want to check object collision to trigger scripts
        if (check_type & Collision_Check_Type::OBJECT) {
            check_type = Collision_Check_Type::OBJECT;
        } else {
            return Collision_Record(Collision_Type::NO_MOVE);
        }
    }

    // Check if passable ahead
    auto map = game.get_map();
    auto collision = map->passable(
        *this,
        movement ? move_dir : direction,
        check_type
    );

    // Move object
    if (collision.passable()) {
        position += change;
    } else {
        if (multiple_directions) {
            // Check if we can move in either direction
            collision = map->passable(*this, move_dir & (Direction::UP | Direction::DOWN),
                check_type, std::move(collision));
            if (collision.passable()) {
                change.x = 0.0f;
            }
            else {
                collision = map->passable(*this, move_dir & (Direction::LEFT | Direction::RIGHT),
                    check_type, std::move(collision));
                if (collision.passable()) {
                    change.y = 0.0f;
                }
            }
        }
        if (collision.passable()) {
            position += change;
            move_dir = vector_to_direction(change);
        } else {
            if (movement && change_facing) direction = move_dir;
            set_state(face_state);
            return collision;
        }
    }

    // Update movement pose
    if (movement) {
        if (change_facing) {
            if (check_close(change.y, -pixels)) {
                direction = Direction::UP;
            } else if (check_close(change.y, pixels)) {
                direction = Direction::DOWN;
            } else if (check_close(change.x, -pixels)) {
                direction = Direction::LEFT;
            } else if (check_close(change.x, pixels)) {
                direction = Direction::RIGHT;
            } else {
                direction = move_dir;
                set_state(face_state);
                return collision;
            }
        }
        set_state(walk_state);
        map->set_objects_moved(true);
        for (auto obj : linked_objects) {
            obj->move(move_dir, pixels, check_type, change_facing);
        }
    }
    return collision;
}

void Map_Object::set_name(const std::string& new_name) {
    name = new_name;
    string_utilities::capitalize(name);
}

std::string Map_Object::prepare_script(const std::string& script) const {
    if (script.empty()) return script;
    auto result = game.get_object_script_preamble();
    auto extension = script.substr(script.find_last_of(".") + 1);
    return result + (extension == "lua" ? file_utilities::read_file(script) : script);
}

void  Map_Object::set_trigger_script(const std::string& script) {
    trigger_script = prepare_script(script);
}

void  Map_Object::set_touch_script(const std::string& script) {
    touch_script = prepare_script(script);
}

void  Map_Object::set_leave_script(const std::string& script) {
    leave_script = prepare_script(script);
}

xd::vec2 Map_Object::get_sprite_magnification() const {
    if (sprite)
        return sprite->get_frame().magnification;
    else
        return xd::vec2(1.0f, 1.0f);
}

xd::vec2 Map_Object::get_text_position() const
{
    auto box = get_bounding_box();
    auto offset = xd::vec2{
        box.x + box.w / 2,
        -game.get_font_style().line_height() / 2
    };
    return get_position() + offset;
}

void Map_Object::set_outlined(std::optional<bool> new_outlined) {
    if (!new_outlined.has_value()) {
        outline_conditions = get_default_outline_conditions();
    } else if (new_outlined.value()) {
        outline_conditions = Outline_Condition::NONE;
    } else {
        outline_conditions = Outline_Condition::NEVER;
    }
}

bool Map_Object::is_outlined() const {
    if ((outline_conditions & Outline_Condition::NEVER) != Outline_Condition::NONE) return false;

    auto result = true;

    if ((outline_conditions & Outline_Condition::TOUCHED) != Outline_Condition::NONE) {
        auto player = game.get_player();
        result = player && (player->get_collision_object() == this ||
            player->get_collision_area() == this);
    }

    if ((outline_conditions & Outline_Condition::SOLID) != Outline_Condition::NONE) {
        result = result && !passthrough;
    }

    if ((outline_conditions & Outline_Condition::SCRIPT) != Outline_Condition::NONE) {
        result = result && !trigger_script.empty();
    }

    result = result || (outlining_object && outlining_object->is_outlined());

    return result;
}

Map_Object::Outline_Condition Map_Object::get_default_outline_conditions() const {
    return Outline_Condition::TOUCHED | Outline_Condition::SOLID | Outline_Condition::SCRIPT;
}

void Map_Object::set_sprite(Game& game, const std::string& filename, const std::string& new_pose_name) {
    if (!file_utilities::file_exists(filename)) {
        LOGGER_W << "Tried to set sprite for map object " << name <<
                    " to nonexistent file " << filename;
        return;
    }
    if (sprite) {
        auto normalized_filename{filename};
        file_utilities::normalize_slashes(normalized_filename);
        if (sprite->get_filename() == normalized_filename)
            return;
        del_component(sprite);
    }
    sprite = std::make_shared<Sprite>(game,
        Sprite_Data::load(game.get_asset_manager(), filename));
    add_component(sprite);
    set_pose(new_pose_name);
}

int Map_Object::get_angle() const {
    return sprite ? sprite->get_frame().angle : 0;
}

void Map_Object::set_angle(int angle) {
    if (!sprite) return;
    sprite->get_frame().angle = angle;
}

float Map_Object::get_speed() const {
    return speed * Configurations::get<int>("debug.logic-fps") / 60.0f;
}

void Map_Object::set_speed(float new_speed) {
    speed = new_speed * 60.0f / Configurations::get<int>("debug.logic-fps");
    if (sprite)
        sprite->set_speed(new_speed);
}

void Map_Object::face(float x, float y) {
    face(xd::vec2(x, y));
}

void Map_Object::face(const Map_Object& other) {
    face(other.get_real_position());
}

void Map_Object::face(xd::vec2 other_position) {
    set_pose("", "", facing_direction(get_real_position(), other_position));
}

void Map_Object::face(Direction dir) {
    if (dir == Direction::FORWARD || dir == Direction::BACKWARD)
        return;
    set_pose("", "", static_cast<Direction>(dir));
}

void Map_Object::run_script(const std::string& script) {
    if (script.empty())
        return;

    if (script_context == Script_Context::GLOBAL)
        game.run_script(script);
    else
        game.get_map()->run_script(script);
}

rapidxml::xml_node<>* Map_Object::save(rapidxml::xml_document<>& doc) {
    auto node = xml_node(doc, "object");
    node->append_attribute(xml_attribute(doc, "id", std::to_string(get_id())));
    if (!get_name().empty())
        node->append_attribute(xml_attribute(doc, "name", get_name()));
    if (!get_type().empty())
        node->append_attribute(xml_attribute(doc, "type", get_type()));
    node->append_attribute(xml_attribute(doc, "x", std::to_string((int)get_x())));
    node->append_attribute(xml_attribute(doc, "y", std::to_string((int)get_y())));
    if (size.x != 0)
        node->append_attribute(xml_attribute(doc, "width", std::to_string((int)size.x)));
    if (size.y != 0)
        node->append_attribute(xml_attribute(doc, "height", std::to_string((int)size.y)));
    if (get_gid() != -1)
        node->append_attribute(xml_attribute(doc, "gid", std::to_string(get_gid())));
    if (!is_visible())
        node->append_attribute(xml_attribute(doc, "visible", "0"));
    properties.save(doc, *node);
    return node;
}

std::unique_ptr<Map_Object> Map_Object::load(rapidxml::xml_node<>& node, Game& game) {
    auto object_ptr = std::make_unique<Map_Object>(game);

    if (auto id_node = node.first_attribute("id"))
        object_ptr->id = std::stoi(id_node->value());

    if (auto name_node = node.first_attribute("name"))
        object_ptr->set_name(name_node->value());

    if (auto type_node = node.first_attribute("type"))
        object_ptr->set_type(type_node->value());

    object_ptr->position.x = std::stof(node.first_attribute("x")->value());
    object_ptr->position.y = std::stof(node.first_attribute("y")->value());

    if (auto width_node = node.first_attribute("width"))
        object_ptr->size.x = std::stof(width_node->value());
    if (auto height_node = node.first_attribute("height"))
        object_ptr->size.y= std::stof(height_node->value());

    if (auto gid_node = node.first_attribute("gid"))
        object_ptr->gid = std::stoi(gid_node->value());

    if (auto visible_node = node.first_attribute("visible"))
        object_ptr->visible = string_utilities::string_to_bool(visible_node->value());

    auto& properties = object_ptr->properties;
    properties.read(node);

    if (properties.has_property("sprite"))
        object_ptr->set_sprite(game, properties["sprite"]);
    if (properties.has_property("direction"))
        object_ptr->set_direction(string_to_direction(properties["direction"]));
    if (properties.has_property("pose"))
        object_ptr->pose_name = properties["pose"];
    if (properties.has_property("state"))
        object_ptr->state = properties["state"];
    if (properties.has_property("speed"))
        object_ptr->set_speed(std::stof(properties["speed"]));
    if (properties.has_property("opacity"))
        object_ptr->set_opacity(std::stof(properties["opacity"]));
    if (properties.has_property("face-state"))
        object_ptr->set_face_state(properties["face-state"]);
    if (properties.has_property("walk-state"))
        object_ptr->set_walk_state(properties["walk-state"]);

    if (properties.has_property("script-context")) {
        auto context_string{properties["script-context"]};
        string_utilities::capitalize(context_string);
        auto context = Script_Context::MAP;
        if (context_string == "GLOBAL")
            context = Script_Context::GLOBAL;
        else if (context_string != "MAP")
            LOGGER_W << "Unknown object script context '" << context_string << "' - defaulting to MAP";
        object_ptr->set_script_context(context);
    }

    if (properties.has_property("script"))
        object_ptr->set_trigger_script(properties["script"]);
    else if (properties.has_property("trigger-script"))
        object_ptr->set_trigger_script(properties["trigger-script"]);

    if (properties.has_property("touch-script"))
        object_ptr->set_touch_script(properties["touch-script"]);

    if (properties.has_property("leave-script"))
        object_ptr->set_leave_script (properties["leave-script"]);

    if (properties.has_property("player-facing"))
        object_ptr->set_player_facing(string_utilities::string_to_bool(properties["player-facing"]));
    if (properties.has_property("passthrough"))
        object_ptr->set_passthrough(string_utilities::string_to_bool(properties["passthrough"]));
    if (properties.has_property("passthrough-type")) {
        auto type_string{properties["passthrough-type"]};
        string_utilities::capitalize(type_string);
        auto type = Passthrough_Type::BOTH;
        if (type_string == "INITIATOR")
            type = Passthrough_Type::INITIATOR;
        else if (type_string == "RECEIVER")
            type = Passthrough_Type::RECEIVER;
        else if (type_string != "BOTH")
            LOGGER_W << "Unknown object passthrough type '" << type_string << "' - defaulting to BOTH";
        object_ptr->set_passthrough_type(type);
    }
    if (properties.has_property("override-tile-collision"))
        object_ptr->set_passthrough(string_utilities::string_to_bool(properties["override-tile-collision"]));
    if (properties.has_property("outlined")) {
        auto outlined{properties["outlined"]};
        string_utilities::capitalize(outlined);
        if (outlined == "TRUE") {
            object_ptr->set_outlined(true);
        } else if (outlined == "FALSE") {
            object_ptr->set_outlined(false);
        } else {
            auto parts = string_utilities::split(outlined, ",");
            if (parts.empty()) {
                LOGGER_W << "Unknown object outlined '" << outlined << "' - defaulting to TOUCHED,SOLID,SCRIPT";
            } else {
                auto conditions = Outline_Condition::NONE;
                for (auto& part : parts) {
                    if (part == "TOUCHED") {
                        conditions = conditions | Outline_Condition::TOUCHED;
                    } else if (part == "SOLID") {
                        conditions = conditions | Outline_Condition::SOLID;
                    } else if (part == "SCRIPT") {
                        conditions = conditions | Outline_Condition::SCRIPT;
                    } else {
                        LOGGER_W << "Unknown object outlined '" << outlined << "' - defaulting to TOUCHED,SOLID,SCRIPT";
                        conditions = object_ptr->get_default_outline_conditions();
                        break;
                    }
                }
                object_ptr->set_outline_conditions(conditions);
            }
        }
    }
    if (properties.has_property("outlined-object")) {
        object_ptr->set_outlined_object_id(std::stoi(properties["outlined-object"]));
    }
    if (properties.has_property("draw-order")) {
        auto order{properties["draw-order"]};
        string_utilities::capitalize(order);
        if (order == "BELOW")
            object_ptr->set_draw_order(Draw_Order::BELOW);
        else if (order == "ABOVE")
            object_ptr->set_draw_order(Draw_Order::ABOVE);
        else if (order != "NORMAL") 
            LOGGER_W << "Unknown object draw order '" << order << "' - defaulting to NORMAL";
    }

    object_ptr->set_pose();

    return object_ptr;
}
