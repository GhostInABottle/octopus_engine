#include "../include/map_object.hpp"
#include "../include/game.hpp"
#include "../include/map.hpp"
#include "../include/sprite.hpp"
#include "../include/sprite_data.hpp"
#include "../include/utility/file.hpp"
#include "../include/utility/xml.hpp"
#include "../include/exceptions.hpp"
#include "../include/direction_utilities.hpp"
#include "../include/log.hpp"
#include "../include/utility/math.hpp"

Map_Object::Map_Object(Game& game, const std::string& name,
        std::string sprite_file, xd::vec2 pos, Direction dir) :
        game(game),
        layer(nullptr),
        id(-1),
        color(1.0f),
        magnification(1.0f),
        gid(0),
        opacity(1.0f),
        visible(true),
        disabled(false),
        stopped(false),
        frozen(false),
        passthrough(false),
        override_tile_collision(false),
        speed(1.0f),
        name(name),
        position(pos),
        state("FACE"),
        direction(dir),
        face_state("FACE"),
        walk_state("WALK"),
        script_context(Script_Context::MAP),
        player_facing(true),
        collision_object(nullptr),
        collision_area(nullptr),
        triggered_object(nullptr),
        draw_order(Draw_Order::NORMAL) {
    if (!sprite_file.empty()) {
        set_sprite(game, sprite_file);
    }
    auto bounding_box = get_bounding_box();
    position.x -= bounding_box.x;
    position.y -= bounding_box.y;
}

Collision_Record Map_Object::move(Direction move_dir, float pixels,
        Collision_Check_Types check_type, bool change_facing) {
    // Map relative directions
    if (move_dir == Direction::FORWARD)
        move_dir = direction;
    else if (move_dir == Direction::BACKWARD)
        move_dir = opposite_direction(direction);

    xd::vec2 movement_vector = direction_to_vector(move_dir);
    xd::vec2 change = movement_vector * pixels;
    bool x_changed = !check_close(change.x, 0.0f);
    bool y_changed = !check_close(change.y, 0.0f);
    bool multiple_directions = x_changed && y_changed;
    bool movement = x_changed || y_changed;

    if (!movement) {
        update_state(face_state);
        // If there was no movement there's no need to check tile collision,
        // but maybe we want to check object collision to trigger scripts
        if (check_type & Collision_Check_Types::OBJECT) {
            check_type = Collision_Check_Types::OBJECT;
        } else {
            return Collision_Record(Collision_Types::NO_MOVE);
        }
    }

    // Check if passable ahead
    auto map = game.get_map();
    auto collision = map->passable(
        *this,
        movement ? move_dir : direction,
        check_type
    );

    // Check suggested direction (e.g. corrections for doors)
    auto edge_dir = collision.edge_direction;
    auto corrected_dir = Direction::NONE;
    if (edge_dir != Direction::NONE && movement && !multiple_directions && !collision.passable()) {
        auto edge_vector = direction_to_vector(edge_dir);
        auto corrected_vector = xd::normalize(movement_vector + edge_vector);
        corrected_dir = vector_to_direction(corrected_vector);
        if (corrected_dir != Direction::NONE) {
            collision = map->passable(*this,
                corrected_dir, check_type);
            if (collision.passable()) {
                change = direction_to_vector(corrected_dir) * pixels;
            } else {
                corrected_dir = Direction::NONE;
            }
        }
    }

    // Move object
    if (collision.passable()) {
        position += change;
    } else {
        if (multiple_directions) {
            // Check if we can move in either direction
            collision = map->passable(*this,
                move_dir & (Direction::UP | Direction::DOWN), check_type);
            if (collision.passable()) {
                change.x = 0.0f;
            }
            else {
                collision = map->passable(*this,
                    move_dir & (Direction::LEFT | Direction::RIGHT), check_type);
                if (collision.passable()) {
                    change.y = 0.0f;
                }
            }
        }
        if (collision.passable()) {
            position += change;
            move_dir = vector_to_direction(change);
        } else {
            if (movement && change_facing)
                direction = move_dir;
            update_state(face_state);
            return collision;
        }
    }

    // Update movement pose
    if (movement) {
        if (change_facing) {
            if (corrected_dir != Direction::NONE) {
                direction = move_dir;
            } else if (check_close(change.y, -pixels)) {
                direction = Direction::UP;
            } else if (check_close(change.y, pixels)) {
                direction = Direction::DOWN;
            } else if (check_close(change.x, -pixels)) {
                direction = Direction::LEFT;
            } else if (check_close(change.x, pixels)) {
                direction = Direction::RIGHT;
            } else {
                direction = move_dir;
                update_state(face_state);
                return collision;
            }
        }
        update_state(walk_state);
        map->set_objects_moved(true);
        return collision;
    }
    return collision;
}

void Map_Object::set_name(const std::string& new_name) {
    name = new_name;
    capitalize(name);
}

void  Map_Object::set_trigger_script(const std::string& script) {
    auto extension = script.substr(script.find_last_of(".") + 1);
    trigger_script = extension == "lua" ? read_file(script) : script;
}

void  Map_Object::set_touch_script(const std::string& script) {
    auto extension = script.substr(script.find_last_of(".") + 1);
    touch_script = extension == "lua" ? read_file(script) : script;
}

void  Map_Object::set_leave_script(const std::string& script) {
    auto extension = script.substr(script.find_last_of(".") + 1);
    leave_script = extension == "lua" ? read_file(script) : script;
}

xd::vec2 Map_Object::get_sprite_magnification() const {
    auto sprite = get_sprite();
    if (sprite)
        return sprite->get_frame().magnification;
    else
        return xd::vec2(1.0f, 1.0f);
}

bool Map_Object::is_outlined() const {
    if (outlined.has_value()) {
        return outlined.value();
    }

    auto player = game.get_player();
    return player
        && player->get_collision_object() == this
        && !passthrough
        && !trigger_script.empty();
}

void Map_Object::set_sprite(Game& game, const std::string& filename, const std::string& new_pose_name) {
    if (!file_exists(filename)) {
        LOGGER_W << "Tried to set sprite for map object " << name <<
                    " to nonexistent file " << filename;
        return;
    }
    if (sprite) {
        auto normalized_filename = filename;
        normalize_slashes(normalized_filename);
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
    if (sprite)
        return get_sprite()->get_frame().angle;
    else
        return 0;
}

void Map_Object::set_angle(int angle) {
    if (sprite)
        get_sprite()->get_frame().angle = angle;
}

void Map_Object::set_speed(float new_speed) {
    speed = new_speed;
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
    direction = facing_direction(get_real_position(), other_position);
    update_pose();
}

void Map_Object::face(Direction dir) {
    if (dir == Direction::FORWARD || dir == Direction::BACKWARD)
        return;
    direction = static_cast<Direction>(dir);
    update_pose();
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
        object_ptr->visible = visible_node->value() == std::string{"1"};

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
        auto context = properties["script-context"];
        capitalize(context);
        object_ptr->set_script_context(context == "GLOBAL" ? Script_Context::GLOBAL : Script_Context::MAP);
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
        object_ptr->set_player_facing(properties["player-facing"] == "true");
    if (properties.has_property("passthrough"))
        object_ptr->set_passthrough(properties["passthrough"] == "true");
    if (properties.has_property("override-tile-collision"))
        object_ptr->set_passthrough(properties["override-tile-collision"] == "true");
    if (properties.has_property("outlined"))
        object_ptr->set_outlined(properties["outlined"] == "true");

    object_ptr->update_pose();

    return object_ptr;
}
