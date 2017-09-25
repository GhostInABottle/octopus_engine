#include "../include/map_object.hpp"
#include "../include/game.hpp"
#include "../include/map.hpp"
#include "../include/sprite.hpp"
#include "../include/sprite_data.hpp"
#include "../include/utility.hpp"
#include "../include/exceptions.hpp"
#include "../include/direction_utilities.hpp"
#include "../include/log.hpp"
#include <boost/lexical_cast.hpp>

Map_Object::Map_Object(Game& game, std::string name,
        xd::asset_manager* manager, std::string sprite_file,
        xd::vec2 pos, Direction dir) :
        game(game), layer(nullptr), id(-1), color(1.0f), gid(-1), opacity(1.0f),
        visible(true), disabled(false), stopped(false), frozen(false),
        passthrough(false), speed(1), name(name), position(pos), state("FACE"),
        direction(dir), collision_area(nullptr), triggered_object(nullptr),
		draw_order(NORMAL) {
    if (manager && !sprite_file.empty()) {
        set_sprite(game, *manager, sprite_file);
    }
    position.y -= get_bounding_box().y;
}

Collision_Record Map_Object::move(Direction move_dir, float pixels,
        Collision_Check_Types check_type, bool change_facing) {
    float x_change = 0.0f;
    float y_change = 0.0f;

    if (move_dir == Direction::FORWARD)
        move_dir = direction;
    else if (move_dir == Direction::BACKWARD)
        move_dir = opposite_direction(direction);

    if ((move_dir & Direction::UP) != Direction::NONE)
        y_change -= pixels;
    if ((move_dir & Direction::DOWN) != Direction::NONE)
        y_change += pixels;
    if ((move_dir & Direction::RIGHT) != Direction::NONE)
        x_change += pixels;
    if ((move_dir & Direction::LEFT) != Direction::NONE)
        x_change -= pixels;

    bool movement = x_change || y_change;
    if (!movement) {
        update_state("FACE");
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
    // Move object
    if (collision.passable()) {
        position.y += y_change;
        position.x += x_change;
    } else if (x_change && y_change) {
        // If two directions, try only one of them
        collision = map->passable(*this,
            move_dir & (Direction::UP | Direction::DOWN), check_type);
        if (collision.passable()) {
            x_change = 0;
            position.y += y_change;
        } else {
            collision = map->passable(*this,
                move_dir & (Direction::LEFT | Direction::RIGHT), check_type);
            if (collision.passable()) {
                y_change = 0;
                position.x += x_change;
            } else {
                if (change_facing)
                    direction = move_dir;
                update_state("FACE");
                return collision;
            }
        }
    }
    else {
        if (movement && change_facing)
            direction = move_dir;
        update_state("FACE");
        return collision;
    }

    // Update movement pose
    if (movement) {
        if (change_facing) {
            if (y_change == -pixels) {
                direction = Direction::UP;
            }
            else if (y_change == pixels) {
                direction = Direction::DOWN;
            }
            else {
                if (x_change == -pixels) {
                    direction = Direction::LEFT;
                }
                else if (x_change == pixels) {
                    direction = Direction::RIGHT;
                }
                else {
                    direction = move_dir;
                    update_state("FACE");
                    return collision;
                }
            }
        }
        update_state("WALK");
        map->set_objects_moved(true);
        return collision;
    }
    return collision;
}

void Map_Object::set_sprite(Game& game, const std::string& filename, const std::string& pose_name) {
    // Player sprite needs to be persisted across maps
    auto& manager = this == game.get_player() ? game.get_asset_manager() :
        game.get_map()->get_asset_manager();
    set_sprite(game, manager, filename, pose_name);
}

void Map_Object::set_sprite(Game& game, xd::asset_manager& manager, const std::string& filename, const std::string& pose_name) {
    if (!file_exists(filename)) {
        LOGGER_W << "Tried to set sprite for map object " << name <<
                    " to nonexistent file " << filename;
        return;
    }
    if (sprite) {
        if (sprite->get_filename() == normalize_slashes(filename))
            return;
        del_component(sprite);
    }
    sprite = xd::create<Sprite>(game, Sprite_Data::load(manager, filename));
    add_component(sprite);
    set_pose(pose_name);
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

void Map_Object::set_speed(float speed) {
    this->speed = speed;
    if (sprite)
        sprite->set_speed(speed);
}

void Map_Object::face(const Map_Object& other) {
    face(other.position.x, other.position.y);
}

void Map_Object::face(float x, float y) {
    direction = facing_direction(position, xd::vec2(x, y));
    update_pose();
}

void Map_Object::face(Direction dir) {
    if (dir == Direction::FORWARD || dir == Direction::BACKWARD)
        return;
    direction = static_cast<Direction>(dir);
    update_pose();
}

void Map_Object::run_script(const Script& script) {
	if (script.source.empty())
		return;
	if (script.is_global)
		game.run_script(script.source);
	else
		game.get_map()->run_script(script.source);
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
    save_properties(properties, doc, *node);
    return node;
}

std::unique_ptr<Map_Object> Map_Object::load(rapidxml::xml_node<>& node, 
        Game& game, xd::asset_manager& manager) {
    using boost::lexical_cast;
    std::unique_ptr<Map_Object> object_ptr(new Map_Object(game));

	if (auto id_node = node.first_attribute("id"))
		object_ptr->id = lexical_cast<int>(id_node->value());

    if (auto name_node = node.first_attribute("name"))
        object_ptr->name = name_node->value();
    if (auto type_node = node.first_attribute("type"))
        object_ptr->type = type_node->value();

    object_ptr->position.x = lexical_cast<float>(node.first_attribute("x")->value());
    object_ptr->position.y = lexical_cast<float>(node.first_attribute("y")->value());

    if (auto width_node = node.first_attribute("width"))
        object_ptr->size.x = lexical_cast<float>(width_node->value());
    if (auto height_node = node.first_attribute("height"))
        object_ptr->size.y= lexical_cast<float>(height_node->value());

    if (auto gid_node = node.first_attribute("gid"))
        object_ptr->gid = lexical_cast<int>(gid_node->value());

    if (auto visible_node = node.first_attribute("visible"))
        object_ptr->visible = lexical_cast<bool>(visible_node->value());

    Properties& properties = object_ptr->properties;
    read_properties(properties, node);

    if (properties.find("sprite") != properties.end())
        object_ptr->set_sprite(game, manager, properties["sprite"]);
    if (properties.find("direction") != properties.end())
        object_ptr->direction = string_to_direction(properties["direction"]);
    if (properties.find("pose") != properties.end())
        object_ptr->pose_name = properties["pose"];
    if (properties.find("state") != properties.end())
        object_ptr->state = properties["state"];
    if (properties.find("speed") != properties.end())
        object_ptr->set_speed(lexical_cast<float>(properties["speed"]));
    if (properties.find("opacity") != properties.end())
        object_ptr->opacity = lexical_cast<float>(properties["opacity"]);

    auto set_script = [&properties](Map_Object* obj, const std::string& type) {
        auto source = properties[type];
        auto extension = source.substr(source.find_last_of(".") + 1);
        if (extension == "lua")
			source = read_file(source);
		Script* script = &obj->trigger_script;
        if (type.find("exit") != std::string::npos)
			script = &obj->exit_script;
		script->source = source;
        script->is_global = type.find("global") != std::string::npos;
    };
	// Trigger script
    if (properties.find("script") != properties.end())
        set_script(object_ptr.get(), "script");
    else if (properties.find("map-script") != properties.end())
        set_script(object_ptr.get(), "map-script");
    else if (properties.find("global-script") != properties.end())
        set_script(object_ptr.get(), "global-script");
	// Exit script
	if (properties.find("exit-script") != properties.end())
		set_script(object_ptr.get(), "exit-script");
	else if (properties.find("map-exit-script") != properties.end())
		set_script(object_ptr.get(), "map-exit-script");
	else if (properties.find("global-exit-script") != properties.end())
		set_script(object_ptr.get(), "global-exit-script");

    object_ptr->update_pose();

    return object_ptr;
}
