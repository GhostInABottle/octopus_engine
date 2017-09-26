#include "../include/map.hpp"
#include "../include/map_object.hpp"
#include "../include/tile_layer.hpp"
#include "../include/image_layer.hpp"
#include "../include/object_layer.hpp"
#include "../include/layer_renderer.hpp"
#include "../include/layer_updater.hpp"
#include "../include/canvas.hpp"
#include "../include/canvas_renderer.hpp"
#include "../include/canvas_updater.hpp"
#include "../include/tileset.hpp"
#include "../include/game.hpp"
#include "../include/scripting_interface.hpp"
#include "../include/utility.hpp"
#include "../include/direction_utilities.hpp"
#include "../include/exceptions.hpp"
#include "../include/rapidxml_print.hpp"
#include <vector>
#include <unordered_set>
#include <boost/lexical_cast.hpp>
#include <xd/system.hpp>
#include <xd/factory.hpp>
#include <xd/audio.hpp>
#include <fstream>
#include <algorithm>

namespace detail {
	std::string generate_unique_name(std::unordered_set<std::string> names,
			std::string base_name = "UNTITLED") {
		int i = 1;
		base_name = capitalize(base_name);
		std::string name = base_name;
		while (names.find(name) != names.end()) {
			name = base_name + std::to_string(i++);
		}
		return name;
	}
}

Map::Map(Game& game) :
        game(game),
		next_object_id(1),
        scripting_interface(new Scripting_Interface(game)),
        collision_tileset(nullptr),
        collision_layer(nullptr),
        needs_redraw(true),
        objects_moved(true) {
    add_component(xd::create<Map_Renderer>());
    add_component(xd::create<Map_Updater>());
    add_component(xd::create<Canvas_Renderer>(game.width(), game.height()));
    add_component(xd::create<Canvas_Updater>());
}

Map::~Map() {}

void Map::run_script(const std::string& script) {
    game.set_current_scripting_interface(scripting_interface.get());
    scripting_interface->run_script(script);
}

void Map::run_startup_scripts() {
    scripting_interface->set_globals();
    for (auto& script : start_scripts) {
        run_script(script);
    }
}

Collision_Record Map::passable(const Map_Object& object, Direction direction,
        Collision_Check_Types check_type) {
    return passable(object, direction, object.get_position(),
        object.get_speed(), check_type);
}

Collision_Record Map::passable(const Map_Object& object, Direction direction,
        xd::vec2 position, float speed, Collision_Check_Types check_type) {
    Collision_Record result;
    if (object.is_passthrough())
        return result;
    auto bounding_box = object.get_bounding_box();
    if (bounding_box.w < 1 || bounding_box.h < 1)
        return result;
    float x_change = (direction & Direction::RIGHT) != Direction::NONE ?
            speed : (direction & Direction::LEFT) != Direction::NONE ?
            -speed : 0.0f;
    float y_change = (direction & Direction::DOWN) != Direction::NONE ?
            speed : (direction & Direction::UP) != Direction::NONE ?
            -speed : 0.0f;
    xd::rect this_box(
        position.x + x_change + bounding_box.x, 
        position.y + y_change + bounding_box.y,
        bounding_box.w,
        bounding_box.h
    );
    Map_Object* area = nullptr;
    // Check object collisions
    if (check_type & Collision_Check_Types::OBJECT) {
        Collision_Record obj_result(Collision_Types::NONE, &object);
        for (auto& object_pair : objects) {
            auto other_id = object_pair.first;
            auto other_object = object_pair.second;
            // Skip self and invisible/passthrough objects
            if (other_id == object.get_id() || !other_object->is_visible() ||
                    other_object->is_passthrough())
                continue;
            // Skip objects with no bounding box
            auto box = other_object->get_bounding_box();
            if (box.w < 1 || box.h < 1)
                continue;
            xd::vec2 other_position = other_object->get_position();
            xd::rect object_box(other_position.x + box.x,
                other_position.y + box.y,
                box.w, box.h);
            if (object_box.w > 0 && object_box.h > 0 &&
                    this_box.intersects(object_box)) {
                std::string other_type = other_object->get_type();
                if (other_type == "area" || other_type == "area object") {
                    area = other_object.get();
                } else {
                    int multi = check_type & Collision_Check_Types::MULTI;
                    obj_result.set(Collision_Types::OBJECT, other_object.get());
                    if (multi) {
                        obj_result.other_objects[other_object->get_name()] = other_object.get();
                    } else
                        return obj_result;
                }
                    
            }
        }
        if (obj_result.type == Collision_Types::OBJECT)
            return obj_result;
    }
    if (check_type & Collision_Check_Types::TILE) {
        // Check tile collisions
        int min_x = static_cast<int>(this_box.x / tile_width);
        int min_y = static_cast<int>(this_box.y / tile_height);
        int max_x = static_cast<int>((this_box.x + this_box.w) / tile_width);
        int max_y = static_cast<int>((this_box.y + this_box.h) / tile_height);
        for (int y = min_y; y <= max_y; ++y) {
            for (int x = min_x; x <= max_x; ++x) {
                // Check map bounds
                if (x < 0 || x >= width || y < 0 || y >= height)
                    return Collision_Record(Collision_Types::TILE);
                // Check if tile is blocking
                if (collision_layer && collision_tileset) {
                    int tile = collision_layer->tiles[x + y * width] -
                        collision_tileset->first_id;
                    if (tile == 2)
                        return Collision_Record(Collision_Types::TILE);
                }
            }
        }
    }
    if (area) {
        auto type = area->get_type() == "area" ?
            Collision_Types::AREA : Collision_Types::AREA_OBJECT;
        return Collision_Record(type, &object, area);
    } else {
         return result;
    }
}

bool Map::tile_passable(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height)
        return false;
    if (!collision_layer || !collision_tileset) return true;
    int tile_index = x + y * width;
    return collision_layer->tiles[tile_index] - collision_tileset->first_id <= 1;
}

int Map::object_count() {
    return objects.size();
}

Map_Object* Map::add_object(Map_Object* object, int layer_index, Object_Layer* layer) {
    return add_object(Object_Ptr(object), layer_index, layer);
}

Map_Object* Map::add_object(Object_Ptr object, int layer_index, Object_Layer* layer) {
	// If layer isn't specified try getting a layer named "objects",
	// if none is found use the 'middle' object layer
    if (!layer) {
        if (layer_index < 0) {
            layer = static_cast<Object_Layer*>(get_layer("objects"));
            if (!layer) {
                layer_index = static_cast<int>(std::floor(
                    object_layers.size() / 2.0));
                layer = object_layers[layer_index];
            }
		}
		else if (static_cast<unsigned>(layer_index) > object_layers.size()) {
			layer = *object_layers.begin();
		}
		else {
			layer = object_layers[layer_index];
		}
    }
    object->set_layer(layer);
	// Update ID counter for new objects
	int id = object->get_id();
	if (id == -1) {
		id = next_object_id;
		object->set_id(id);
	}
	if (id >= next_object_id) {
		next_object_id = id + 1;
	}
	// Add to object map and to layer
	auto name = capitalize(object->get_name());
	auto mapping = std::unordered_multimap<std::string, int>::value_type(name, id);
	object_name_to_id.insert(mapping);
	objects[id] = object;
    layer->objects.push_back(object.get());
    return object.get();
}

Map_Object* Map::add_object(std::string name, std::string sprite_file,
	xd::vec2 pos, Direction dir) {
	auto object_ptr = new Map_Object(game, name, &asset_manager, sprite_file, pos, dir);
	add_object(object_ptr);
	if (name.empty())
		object_ptr->set_name("UNTITLED" + object_ptr->get_id());
	return object_ptr;
}

Map_Object* Map::get_object(const std::string& name) {
    auto cap_name = capitalize(name);
	if (object_name_to_id.find(cap_name) != object_name_to_id.end()) {
		int id = object_name_to_id.find(cap_name)->second;
		return get_object(id);
	}
	return nullptr;
}

Map_Object* Map::get_object(int id) {
	if (objects.find(id) != objects.end())
		return objects[id].get();
	else
		return nullptr;
}

void Map::delete_object(const std::string& name) {
    delete_object(get_object(name));
}

void Map::delete_object(int id) {
	delete_object(get_object(id));
}

void Map::delete_object(Map_Object* object) {
    if (!object)
        return;
    auto& layer_objects = object->get_layer()->objects;
    layer_objects.erase(
        std::remove(layer_objects.begin(), layer_objects.end(), object),
        layer_objects.end()
    );
	erase_object_references(object);
}

void Map::erase_object_references(Map_Object* object) {
	object_name_to_id.erase(capitalize(object->get_name()));
	objects.erase(object->get_id());
}

int Map::layer_count() {
    return layers.size();
}

Layer* Map::get_layer(int id) {
    if (id >= 1 && id <= layer_count())
        return layers[id - 1].get();
    else
        return nullptr;
}

Layer* Map::get_layer(const std::string& name) {
    auto cap_name = capitalize(name);
    auto layer = std::find_if(layers.begin(), layers.end(),
        [&cap_name](std::shared_ptr<Layer> layer) {
            return capitalize(layer->name) == cap_name;
    });
    if (layer != layers.end())
        return layer->get();
    else
        return nullptr;
}

void Map::add_layer(Layer_Types type) {
	std::shared_ptr<Layer> layer;
	switch (type) {
	case Layer_Types::OBJECT:
		layer = std::make_shared<Object_Layer>();
		break;
	case Layer_Types::IMAGE:
		layer = std::make_shared<Image_Layer>();
		break;
	case Layer_Types::TILE:
		layer = std::make_shared<Tile_Layer>();
		break;
	default:
		return;
	}
	std::unordered_set<std::string> names;
	for (auto& layer : layers) {
		names.insert(layer->name);
	}
	layer->name = detail::generate_unique_name(names);
	layers.push_back(layer);
    if (type == Layer_Types::OBJECT)
        object_layers.push_back((Object_Layer*)layer.get());
}

void Map::delete_layer(const std::string& name) {
    auto cap_name = capitalize(name);
    // Delete any matching object layer and its object
    for (auto layer = object_layers.begin(); layer != object_layers.end();)
    {
        if (capitalize((*layer)->name) != cap_name)
        {
            layer++;
        } else {
            auto obj_layer = static_cast<Object_Layer*>(*layer);
            for (auto& obj : obj_layer->objects)
            {
				erase_object_references(obj);
            }
            layer = object_layers.erase(layer);
        }
    }
    // Clear the collision object if necessary
    if (collision_layer && capitalize(collision_layer->name) == cap_name)
        collision_layer = nullptr;
    // Delete matching layers
    layers.erase(std::remove_if(layers.begin(), layers.end(),
        [&cap_name](std::shared_ptr<Layer> layer) {
              return capitalize(layer->name) == cap_name;
        }
    ), layers.end());
}

void Map::resize(xd::ivec2 map_size, xd::ivec2 tile_size) {
    if (map_size == xd::ivec2(width, height) &&
            tile_size == xd::ivec2(tile_width, tile_height))
        return;
    this->width = map_size.x;
    this->height = map_size.y;
    this->tile_width = tile_size.x;
    this->tile_height = tile_size.y;
    for (auto& layer : layers) {
        layer->resize(map_size);
    }
    needs_redraw = true;
}

void Map::save(std::string filename) {
    rapidxml::xml_document<> doc;
    auto decl_node = xml_node(doc, "", "", rapidxml::node_declaration);
    decl_node->append_attribute(xml_attribute(doc, "version", "1.0"));
    decl_node->append_attribute(xml_attribute(doc, "encoding", "UTF-8"));
    doc.append_node(decl_node);
    auto map_node = save(doc);
    doc.append_node(map_node);
    std::ofstream out;
    out.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
    out << doc;
}

rapidxml::xml_node<>* Map::save(rapidxml::xml_document<>& doc) {
    auto node = xml_node(doc, "map");
    node->append_attribute(xml_attribute(doc, "version", "1.0"));
    node->append_attribute(xml_attribute(doc, "orientation", "orthogonal"));
    node->append_attribute(xml_attribute(doc, "width", std::to_string(width)));
    node->append_attribute(xml_attribute(doc, "height", std::to_string(height)));
    node->append_attribute(xml_attribute(doc, "tilewidth", std::to_string(tile_width)));
    node->append_attribute(xml_attribute(doc, "tileheight", std::to_string(tile_height)));
    save_properties(properties, doc, *node);
    // Tilesets
    for (auto& tileset : tilesets) {
        auto tileset_node = tileset.save(doc);
        node->append_node(tileset_node);
    }
    // Layers
    for (auto& layer : layers) {
        auto layer_node = layer->save(doc);
        node->append_node(layer_node);
    }
    return node;
}

std::unique_ptr<Map> Map::load(Game& game, const std::string& filename) {
    rapidxml::xml_document<> doc;
    char* content = doc.allocate_string(read_file(filename).c_str());
    doc.parse<0>(content);
    auto map_node = doc.first_node("map");
    if (!map_node)
        throw tmx_exception("Invalid TMX file. Missing map node");
    auto map = load(game, *map_node);
    map->filename = normalize_slashes(filename);
    return map;
}

std::unique_ptr<Map> Map::load(Game& game, rapidxml::xml_node<>& node) {
    using boost::lexical_cast;
    std::unique_ptr<Map> map_ptr(new Map(game));

    if (node.first_attribute("orientation")->value() != std::string("orthogonal"))
        throw tmx_exception("Invalid map orientation, expected orthogonal");

    map_ptr->width = lexical_cast<int>(node.first_attribute("width")->value());
    map_ptr->height = lexical_cast<int>(node.first_attribute("height")->value());
    map_ptr->tile_width = lexical_cast<int>(node.first_attribute("tilewidth")->value());
    map_ptr->tile_height = lexical_cast<int>(node.first_attribute("tileheight")->value());

    // Map properties
    read_properties(map_ptr->properties, node);

    // Background music
    if (map_ptr->properties.find("music") != map_ptr->properties.end())
        map_ptr->background_music = map_ptr->properties["music"];

    // Startup scripts
    if (map_ptr->properties.find("scripts") != map_ptr->properties.end()) {
        auto filenames = split(map_ptr->properties["scripts"], ",");
        for (auto& filename : filenames) {
            map_ptr->start_scripts.push_back(read_file(trim(filename)));
        }
    }

    // Tilesets
    for (auto tileset_node = node.first_node("tileset");
            tileset_node; tileset_node = tileset_node->next_sibling("tileset")) {
        std::unique_ptr<Tileset> tileset_ptr;
        if (auto source_node = tileset_node->first_attribute("source")) {
            std::string source = source_node->value();
            tileset_ptr = Tileset::load(source);
			tileset_ptr->first_id = lexical_cast<int>(
				tileset_node->first_attribute("firstgid")->value());
        } else {
            tileset_ptr = Tileset::load(*tileset_node);
        }
        map_ptr->tilesets.push_back(*tileset_ptr);
        if (tileset_ptr->name == "collision")
                map_ptr->collision_tileset = &(map_ptr->tilesets[map_ptr->tilesets.size() - 1]);
    }

    // Layers
    rapidxml::xml_node<>* layer_node = node.first_node();
    while (layer_node) {
        std::shared_ptr<Layer> layer;
        std::string node_name(layer_node->name());
        if (node_name == "layer") {
            layer = std::shared_ptr<Layer>(Tile_Layer::load(*layer_node, *game.get_camera()));
            if (layer->name == "collision") {
                layer->visible = false;
                map_ptr->collision_layer = static_cast<Tile_Layer*>(layer.get());
            }
        }
        else if (node_name == "imagelayer") {
            layer = std::shared_ptr<Layer>(Image_Layer::load(*layer_node, game,
                *game.get_camera(), map_ptr->asset_manager));
        }
        else if (node_name == "objectgroup") {
            layer = std::shared_ptr<Layer>(Object_Layer::load(*layer_node, game, *game.get_camera(), *map_ptr));
            map_ptr->object_layers.push_back(static_cast<Object_Layer*>(layer.get()));
        }

        if (layer.get())
            map_ptr->layers.push_back(layer);

        layer_node = layer_node->next_sibling();
    }

    if (map_ptr->object_layers.empty())
        throw tmx_exception("Must have at least one object layer in map");

    return map_ptr;
}

void Map_Renderer::render(Map& map) {
    for (auto& layer : map.layers) {
        if (auto renderer = layer->renderer.get()) {
            if (map.needs_redraw)
                renderer->redraw();
            renderer->render(map);
        }
    }
    map.needs_redraw = false;
}

void Map_Updater::update(Map& map) {
    map.game.set_current_scripting_interface(map.scripting_interface.get());
    map.scripting_interface->update();
    for (auto& layer : map.layers) {
        if (auto updater = layer->updater.get())
            updater->update(map);
    }
}
