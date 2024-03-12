#include "../include/map_object.hpp"
#include "../include/game.hpp"
#include "../include/audio_player.hpp"
#include "../include/map.hpp"
#include "../include/sprite.hpp"
#include "../include/sprite_data.hpp"
#include "../include/utility/direction.hpp"
#include "../include/utility/file.hpp"
#include "../include/utility/math.hpp"
#include "../include/utility/xml.hpp"
#include "../include/utility/string.hpp"
#include "../include/exceptions.hpp"
#include "../include/configurations.hpp"
#include <utility>

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
        strict_multidirectional_movement(false),
        use_layer_color(true),
        direction(dir),
        state("FACE"),
        face_state("FACE"),
        walk_state("WALK"),
        script_context(Script_Context::MAP),
        collision_object(nullptr),
        collision_area(nullptr),
        triggered_object(nullptr),
        draw_order(Draw_Order::NORMAL),
        speed(1.0f),
        sound_attenuation_enabled(false) {
    if (!sprite_file.empty()) {
        set_sprite(game, sprite_file);
    }
    set_speed(1.0f);
}

Map_Object::~Map_Object() {}

Collision_Record Map_Object::move(Direction move_dir, float pixels,
        Collision_Check_Type check_type, bool change_facing) {
    // Map relative directions
    if (direction_contains(move_dir, Direction::FORWARD)) {
        move_dir = direction;
    } else if (direction_contains(move_dir, Direction::BACKWARD)) {
        move_dir = opposite_direction(direction);
    }

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
        auto new_dir = movement && change_facing ? move_dir : Direction::NONE;
        auto try_multi_dir = multiple_directions && !strict_multidirectional_movement;
        if (!try_multi_dir) {
            // Can't move due to collision
            set_state_and_direction(face_state, new_dir);
            return collision;
        }

        // Check if we can move in either direction
        Collision_Record one_dir_collision{ collision };
        one_dir_collision = map->passable(*this, move_dir & (Direction::UP | Direction::DOWN), check_type);
        if (one_dir_collision.passable()) {
            change.x = 0.0f;
            change.y = change.y >= 0 ? pixels : -pixels;
        } else if (one_dir_collision = map->passable(*this, move_dir & (Direction::LEFT | Direction::RIGHT), check_type);
                one_dir_collision.passable()) {
            change.y = 0.0f;
            change.x = change.x >= 0 ? pixels : -pixels;
        }

        if (!one_dir_collision.passable()) {
            // Can't move in either direction
            set_state_and_direction(face_state, new_dir);
            return collision;
        }

        // Move in the single passable direction
        position += change;
        // Don't change direction frequently when moving along a circular curve
        auto change_dir = !collision.other_object
            || !collision.other_object->get_bounding_circle();
        if (change_dir) {
            move_dir = vector_to_direction(change);
        } else {
            direction = new_dir;
            change_facing = false;
        }
        collision = one_dir_collision;
    }

    // Update movement pose
    if (!movement)  return collision;

    if (change_facing) {
        if (sprite && sprite->is_eight_directional()) {
            direction = move_dir;
        } else if (change.y < 0) {
            direction = Direction::UP;
        } else if (change.y > 0) {
            direction = Direction::DOWN;
        } else if (change.x < 0) {
            direction = Direction::LEFT;
        } else if (change.x > 0) {
            direction = Direction::RIGHT;
        } else {
            set_state_and_direction(face_state, move_dir);
            return collision;
        }
    }

    set_state_and_direction(walk_state, direction);

    map->set_objects_moved(true);
    for (auto obj : linked_objects) {
        obj->move(move_dir, pixels, check_type, change_facing);
    }

    return collision;
}

void Map_Object::set_name(const std::string& new_name) {
    name = new_name;
    string_utilities::capitalize(name);
}

std::string Map_Object::prepare_script(const std::string& script) const {
    if (script.empty()) return script;

    auto preamble = game.get_object_script_preamble();
    auto extension = script.substr(script.find_last_of(".") + 1);
    if (extension != "lua") return preamble + script;

    auto fs = file_utilities::game_data_filesystem();
    auto script_filename = game.get_scripts_directory() + script;
    return preamble + fs->read_file(script_filename);
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

xd::vec2 Map_Object::get_text_position() const {
    auto offset = xd::vec2{
        bounding_box.x + bounding_box.w / 2,
        -game.get_font_style().line_height() / 2
    };
    return get_position() + offset;
}

xd::vec2 Map_Object::get_size() const {
    if (size.x > 0 || size.y > 0 || !sprite)
        return size;

    return sprite->get_size();
}

void Map_Object::set_size(xd::vec2 new_size) {
    size = new_size;
    if (!bounding_circle) {
        bounding_box = xd::rect{ 0, 0, size[0], size[1] };
        return;
    }

    if (!check_close(size.x, size.y)) {
        throw std::runtime_error("Invalid non-unfirom size for circle object " + name);
    }
    bounding_circle = xd::circle{ size.x / 2, size.y / 2, size.x / 2 };
    bounding_box = static_cast<xd::rect>(bounding_circle.value());
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
    auto fs = file_utilities::game_data_filesystem();
    if (!fs->exists(filename)) {
        throw std::runtime_error("Tried to set sprite for map object " + name +
            " to nonexistent file " + filename);
    }

    if (sprite) {
        auto normalized_filename{filename};
        string_utilities::normalize_slashes(normalized_filename);
        if (sprite->get_filename() == normalized_filename)
            return;
    }

    auto& asset_manager = game.get_asset_manager();
    auto audio = game.get_audio_player().get_audio();
    auto channel_group = game.get_sound_group_type();
    auto new_sprite = std::make_shared<Sprite>(game,
        Sprite_Data::load(filename, asset_manager, audio, channel_group));

    if (sprite) {
        del_component(sprite);
    }

    sprite = new_sprite;
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
    auto logic_fps = Configurations::get<int>("graphics.logic-fps", "debug.logic-fps");
    return speed * logic_fps / 60.0f;
}

void Map_Object::set_speed(float new_speed) {
    auto logic_fps = Configurations::get<int>("graphics.logic-fps", "debug.logic-fps");
    speed = new_speed * 60.0f / logic_fps;
    if (sprite)
        sprite->set_speed(new_speed);
}

void Map_Object::set_pose(const std::string& new_pose_name, const std::string& new_state, Direction new_direction) {
    if (!new_pose_name.empty())
        pose_name = new_pose_name;
    else if (pose_name.empty() && sprite)
        pose_name = sprite->get_default_pose();

    if (!new_state.empty())
        state = new_state;

    if (new_direction != Direction::NONE) {
        direction = new_direction;
        if (is_diagonal(direction) && sprite && !sprite->is_eight_directional()) {
            direction = diagonal_to_four_directions(direction);
        }
    }

    Sprite_Holder::set_pose(pose_name, state, direction);
    for (auto obj : linked_objects) {
        obj->set_pose(new_pose_name, new_state, new_direction);
    }

    if (!sprite) return;

    bounding_box = sprite->get_bounding_box();
    bounding_circle = sprite->get_bounding_circle();
}

void Map_Object::face(float x, float y) {
    face(xd::vec2(x, y));
}

void Map_Object::face(const Map_Object& other) {
    face(other.get_centered_position());
}

void Map_Object::face(xd::vec2 other_position) {
    set_pose("", "", facing_direction(get_centered_position(), other_position));
}

void Map_Object::face(Direction dir) {
    if (is_relative_direction(dir)) return;

    set_pose("", "", static_cast<Direction>(dir));
}

void Map_Object::set_state_and_direction(const std::string& new_state, Direction dir) {
    set_pose("", frozen ? "" : new_state,
        is_relative_direction(dir) ? Direction::NONE : dir);
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
    if (get_gid() != 0)
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
    else
        throw tmx_exception("Missing map object ID");

    if (auto name_node = node.first_attribute("name"))
        object_ptr->set_name(name_node->value());

    if (auto type_node = node.first_attribute("type"))
        object_ptr->set_type(type_node->value());

    if (auto x_node = node.first_attribute("x"))
        object_ptr->position.x = std::stof(x_node->value());
    else
        throw tmx_exception("Missing X coordinate for object with ID " + std::to_string(object_ptr->id));

    if (auto y_node = node.first_attribute("y"))
        object_ptr->position.y = std::stof(y_node->value());
    else
        throw tmx_exception("Missing Y coordinate for object with ID " + std::to_string(object_ptr->id));

    float obj_width, obj_height = 0.0f;

    if (auto width_node = node.first_attribute("width")) {
        obj_width = std::stof(width_node->value());
        object_ptr->size.x = obj_width;
    }
    if (auto height_node = node.first_attribute("height")) {
        obj_height = std::stof(height_node->value());
        object_ptr->size.y = obj_height;
    }

    if (auto gid_node = node.first_attribute("gid"))
        object_ptr->gid = std::stoi(gid_node->value());

    if (auto visible_node = node.first_attribute("visible"))
        object_ptr->visible = string_utilities::string_to_bool(visible_node->value());

    // Object properties
    auto& properties = object_ptr->properties;
    properties.read(node);

    if (properties.contains("sprite"))
        object_ptr->set_sprite(game, properties["sprite"]);
    if (properties.contains("direction"))
        object_ptr->set_direction(string_to_direction(properties["direction"]));
    if (properties.contains("pose"))
        object_ptr->pose_name = properties["pose"];
    if (properties.contains("state"))
        object_ptr->state = properties["state"];
    if (properties.contains("speed"))
        object_ptr->set_speed(std::stof(properties["speed"]));
    if (properties.contains("opacity"))
        object_ptr->set_opacity(std::stof(properties["opacity"]));
    if (properties.contains("face-state"))
        object_ptr->set_face_state(properties["face-state"]);
    if (properties.contains("walk-state"))
        object_ptr->set_walk_state(properties["walk-state"]);

    if (properties.contains("script-context")) {
        auto context_string{properties["script-context"]};
        string_utilities::capitalize(context_string);
        auto context = Script_Context::MAP;

        if (context_string == "GLOBAL")
            context = Script_Context::GLOBAL;
        else if (context_string != "MAP")
            throw tmx_exception("Unknown object script context: " + context_string);

        object_ptr->set_script_context(context);
    }

    if (properties.contains("script"))
        object_ptr->set_trigger_script(properties["script"]);
    else if (properties.contains("trigger-script"))
        object_ptr->set_trigger_script(properties["trigger-script"]);

    if (properties.contains("touch-script"))
        object_ptr->set_touch_script(properties["touch-script"]);

    if (properties.contains("leave-script"))
        object_ptr->set_leave_script(properties["leave-script"]);

    if (properties.contains("passthrough"))
        object_ptr->set_passthrough(string_utilities::string_to_bool(properties["passthrough"]));

    if (properties.contains("passthrough-type")) {
        auto type_string{properties["passthrough-type"]};
        string_utilities::capitalize(type_string);

        auto type = Passthrough_Type::BOTH;
        if (type_string == "INITIATOR")
            type = Passthrough_Type::INITIATOR;
        else if (type_string == "RECEIVER")
            type = Passthrough_Type::RECEIVER;
        else if (type_string != "BOTH")
            throw tmx_exception("Unknown object passthrough type: " + type_string);

        object_ptr->set_passthrough_type(type);
    }

    if (properties.contains("override-tile-collision"))
        object_ptr->set_passthrough(string_utilities::string_to_bool(properties["override-tile-collision"]));

    if (properties.contains("use-layer-color"))
        object_ptr->set_use_layer_color(string_utilities::string_to_bool(properties["use-layer-color"]));

    if (properties.contains("outlined")) {
        auto outlined{properties["outlined"]};
        string_utilities::capitalize(outlined);
        if (outlined == "TRUE") {
            object_ptr->set_outlined(true);
        } else if (outlined == "FALSE") {
            object_ptr->set_outlined(false);
        } else {
            auto parts = string_utilities::split(outlined, ",");
            if (parts.empty()) {
                throw tmx_exception("Unknown object outlined value: " + outlined);
            } else {
                auto conditions = Outline_Condition::NONE;
                for (const auto& part : parts) {
                    auto trimmed = string_utilities::trim(part);
                    if (trimmed == "TOUCHED") {
                        conditions = conditions | Outline_Condition::TOUCHED;
                    } else if (trimmed == "SOLID") {
                        conditions = conditions | Outline_Condition::SOLID;
                    } else if (trimmed == "SCRIPT") {
                        conditions = conditions | Outline_Condition::SCRIPT;
                    } else {
                        throw tmx_exception("Unknown object outlined value: " + outlined);
                    }
                }
                object_ptr->set_outline_conditions(conditions);
            }
        }
    }

    if (properties.contains("outlined-object")) {
        object_ptr->set_outlined_object_id(std::stoi(properties["outlined-object"]));
    }

    if (properties.contains("draw-order")) {
        auto order{properties["draw-order"]};
        string_utilities::capitalize(order);
        if (order == "BELOW")
            object_ptr->set_draw_order(Draw_Order::BELOW);
        else if (order == "ABOVE")
            object_ptr->set_draw_order(Draw_Order::ABOVE);
        else if (order != "NORMAL")
            throw tmx_exception("Unknown object draw order: " + order);
    }

    if (properties.contains("sfx-attenuation")) {
        auto attenuation{properties["sfx-attenuation"]};
        string_utilities::capitalize(attenuation);
        if (attenuation == "TRUE")
            object_ptr->set_sound_attenuation_enabled(true);
        else if (attenuation != "FALSE")
            throw tmx_exception("Invalid object sfx-attenuation value: " + attenuation);
    }

    // Collision box and circle
    if (node.first_node("ellipse")) {
        if (!check_close(obj_width, obj_height)) {
            throw tmx_exception("Only ciecle ellipse objects are supported. "
                + object_ptr->name + " width and height don't match.");
        }
        object_ptr->bounding_circle = xd::circle{ obj_width / 2, obj_height / 2, obj_width / 2 };
    }

    if (object_ptr->sprite) {
        object_ptr->bounding_box = object_ptr->sprite->get_bounding_box();
        auto had_circle = object_ptr->bounding_circle.has_value();
        object_ptr->bounding_circle = object_ptr->sprite->get_bounding_circle();
        if (had_circle && !object_ptr->bounding_circle) {
            throw tmx_exception{ "Trying to use sprite " + object_ptr->sprite->get_filename()
                + " with no bounding circle data on ellipse object " + object_ptr->name };
        }
    } else if (object_ptr->bounding_circle) {
        object_ptr->bounding_box = static_cast<xd::rect>(object_ptr->bounding_circle.value());
    } else {
        object_ptr->bounding_box = xd::rect{ 0, 0, object_ptr->size[0], object_ptr->size[1] };
    }

    object_ptr->set_pose();

    return object_ptr;
}
