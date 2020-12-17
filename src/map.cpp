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
#include "../include/xd/vendor/sol/sol.hpp"
#include "../include/utility/file.hpp"
#include "../include/utility/xml.hpp"
#include "../include/utility/direction.hpp"
#include "../include/exceptions.hpp"
#include "../include/vendor/rapidxml_print.hpp"
#include "../include/xd/system.hpp"
#include "../include/xd/audio.hpp"
#include "../include/configurations.hpp"
#include "../include/log.hpp"
#include <vector>
#include <unordered_set>
#include <fstream>
#include <algorithm>
#include <utility>

namespace detail {
    std::string generate_unique_name(std::unordered_set<std::string> names,
            std::string base_name = "UNTITLED") {
        int i = 1;
        string_utilities::capitalize(base_name);
        std::string name = base_name;
        while (names.find(name) != names.end()) {
            name = base_name + std::to_string(i++);
        }
        return name;
    }
}

Map::Map(Game& game) :
        game(game),
        width(1),
        height(1),
        tile_width(1),
        tile_height(1),
        next_object_id(1),
        scripting_interface(std::make_unique<Scripting_Interface>(game)),
        collision_tileset(nullptr),
        collision_layer(nullptr),
        needs_redraw(true),
        objects_moved(true),
        canvases_sorted(false) {
    add_component(std::make_shared<Map_Renderer>());
    add_component(std::make_shared<Map_Updater>());
    add_component(std::make_shared<Canvas_Renderer>(game, *game.get_camera()));
    add_component(std::make_shared<Canvas_Updater>());
}

Map::~Map() {}

void Map::run_script_impl(const std::string& script_or_filename, bool is_filename) {
    auto old_interface = game.get_current_scripting_interface();
    game.set_current_scripting_interface(scripting_interface.get());
    if (is_filename) {
        scripting_interface->schedule_file(script_or_filename);
    } else {
        scripting_interface->schedule_code(script_or_filename);
    }
    game.set_current_scripting_interface(old_interface);
}

void Map::run_function(const sol::protected_function& function) {
    auto old_interface = game.get_current_scripting_interface();
    game.set_current_scripting_interface(scripting_interface.get());
    scripting_interface->schedule_function(function);
    game.set_current_scripting_interface(old_interface);
}

void Map::run_startup_scripts() {
    LOGGER_I << "Running startup scripts";
    scripting_interface->set_globals();
    auto map_loaded_script = Configurations::get<std::string>("game.map-loaded-script");
    if (!map_loaded_script.empty()) {
        run_script_file(map_loaded_script);
    }
    for (auto& script : start_scripts) {
        run_script_file(script);
    }
}

Collision_Record Map::passable(const Map_Object& object, Direction direction,
        Collision_Check_Type check_type, Collision_Record&& previous_record) {
    return passable(object, direction, object.get_position(),
        object.get_fps_independent_speed(), check_type, std::move(previous_record));
}

Collision_Record Map::passable(const Map_Object& object, Direction direction,
        xd::vec2 position, float speed, Collision_Check_Type check_type,
        Collision_Record&& previous_record) {
    Collision_Record result{std::move(previous_record)};
    result.type = Collision_Type::NONE;

    if (object.initiates_passthrough())
        return result;
    const auto bounding_box = object.get_bounding_box();
    if (bounding_box.w < 1 || bounding_box.h < 1)
        return result;

    const float x_change = (direction & Direction::RIGHT) != Direction::NONE ?
            speed : (direction & Direction::LEFT) != Direction::NONE ?
            -speed : 0.0f;
    const float y_change = (direction & Direction::DOWN) != Direction::NONE ?
            speed : (direction & Direction::UP) != Direction::NONE ?
            -speed : 0.0f;
    xd::rect this_box(
        position.x + x_change + bounding_box.x,
        position.y + y_change + bounding_box.y,
        bounding_box.w,
        bounding_box.h
    );

    bool check_tile_collision = check_type & Collision_Check_Type::TILE;

    // Check object collisions
    if (check_type & Collision_Check_Type::OBJECT) {
        for (auto& object_pair : objects) {
            auto other_id = object_pair.first;
            auto other_object = object_pair.second.get();
            const auto visible = other_object->is_visible();
            const auto passthrough = other_object->receives_passthrough();

            // Skip objects with no bounding box
            const auto box = other_object->get_bounding_box();
            if (box.w < 1 || box.h < 1)
                continue;
            const auto other_pos = other_object->get_position();
            xd::rect object_box{other_pos.x + box.x, other_pos.y + box.y, box.w, box.h};
            const auto intersects = this_box.intersects(object_box);

            // Special case for skipping tile collision detection
            if (other_object->overrides_tile_collision() && visible && passthrough && intersects)
                check_tile_collision = false;
            // Areas are passthrough objects with a script
            const auto is_area = passthrough && other_object->has_any_script();
            // Skip self, invisible, or passthrough objects (except areas)
            if (other_id == object.get_id()|| !visible|| (!is_area && passthrough))
                continue;

            if (intersects) {
                if (is_area) {
                    if (result.type == Collision_Type::NONE)
                        result.type = Collision_Type::AREA;
                    result.other_area = other_object;
                    result.other_areas[other_object->get_name()] = other_object;
                } else {
                    result.type = Collision_Type::OBJECT;
                    // Prefer objects with scripts
                    if (!result.other_object || other_object->has_any_script()) {
                        result.other_object = other_object;
                    }
                    result.other_objects[other_object->get_name()] = other_object;
                    check_tile_collision = false;
                }
            }
        }
    }

    // Check tile collisions (unless an object overrides collision)
    if (check_tile_collision) {
        const xd::vec2 tile_pos{this_box.x / tile_width, this_box.y / tile_height};
        // Minimum map bounds check (before small negatives are cast to 0)
        if (tile_pos.x < 0.0f || tile_pos.y < 0.0f) {
            result.type = Collision_Type::TILE;
            return result;
        }
        const int min_x = static_cast<int>(tile_pos.x);
        const int min_y = static_cast<int>(tile_pos.y);
        const int max_x = static_cast<int>((this_box.x + this_box.w - 1) / tile_width);
        const int max_y = static_cast<int>((this_box.y + this_box.h - 1) / tile_height);
        for (int y = min_y; y <= max_y; ++y) {
            for (int x = min_x; x <= max_x; ++x) {
                // Maximum map bounds check (taking collision box into account)
                if (x >= width || y >= height) {
                    result.type = Collision_Type::TILE;
                    return result;
                }
                // Check if tile is blocking
                if (collision_layer && collision_tileset) {
                    const int tile = collision_layer->tiles[x + y * width] -
                        collision_tileset->first_id;
                    if (tile >= 2) {
                        result.type = Collision_Type::TILE;
                        return result;
                    }
                }
            }
        }
    }

    return result;
}

bool Map::tile_passable(int x, int y) const noexcept {
    if (x < 0 || x >= width || y < 0 || y >= height)
        return false;
    if (!collision_layer || !collision_tileset) return true;
    const int tile_index = x + y * width;
    return collision_layer->tiles[tile_index] - collision_tileset->first_id <= 1;
}

int Map::object_count() const noexcept {
    return objects.size();
}

Map_Object* Map::add_object(const std::shared_ptr<Map_Object>& object, Object_Layer* layer) {
    // If layer isn't specified try getting a layer named "objects",
    // if none is found use the 'middle' object layer
    if (!layer) {
        layer = static_cast<Object_Layer*>(get_layer("objects"));
        if (!layer) {
            int index = static_cast<int>(std::floor(object_layers.size() / 2.0));
            layer = object_layers[index];
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
    auto name = object->get_name();
    if (name.empty()) {
        name = "UNTITLED" + std::to_string(id);
        object->set_name(name);
    }
    string_utilities::capitalize(name);
    auto mapping = std::unordered_multimap<std::string, int>::value_type(name, id);
    object_name_to_id.insert(mapping);
    objects[id] = object;
    object->set_name(name);
    layer->objects.push_back(object.get());
    return object.get();
}

Map_Object* Map::add_new_object(std::optional<std::string> name, std::optional<std::string> sprite_file,
        std::optional<xd::vec2> pos, std::optional<Direction> dir, std::optional<Object_Layer*> layer) {
    return add_object(std::make_shared<Map_Object>(game, name.value_or(""), sprite_file.value_or(""),
        pos.value_or(xd::vec2{}), dir.value_or(Direction::DOWN)), layer.value_or(nullptr));
}

Map_Object* Map::get_object(std::string name) const {
    string_utilities::capitalize(name);
    if (object_name_to_id.find(name) != object_name_to_id.end()) {
        int id = object_name_to_id.find(name)->second;
        return get_object(id);
    }
    return nullptr;
}

Map_Object* Map::get_object(int id) const {
    auto obj = objects.find(id);
    if (obj != objects.end())
        return obj->second.get();
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
    auto player = game.get_player();
    if (object == player->get_triggered_object()) {
        player->set_triggered_object(nullptr);
    }
    if (object == player->get_collision_object()) {
        player->set_collision_object(nullptr);
    }
    if (object == player->get_collision_area()) {
        player->set_collision_area(nullptr);
    }
    if (object == player->get_outlining_object()) {
        player->set_outlining_object(nullptr);
    }
    auto name = object->get_name();
    string_utilities::capitalize(name);
    object_name_to_id.erase(name);
    objects.erase(object->get_id());
}

int Map::layer_count() const {
    return layers.size();
}

Layer* Map::get_layer(int id) const {
    if (id >= 1 && id <= layer_count())
        return layers[id - 1].get();
    else
        return nullptr;
}

Layer* Map::get_layer(std::string name) const {
    string_utilities::capitalize(name);
    auto layer = std::find_if(layers.begin(), layers.end(),
        [&name](std::shared_ptr<Layer> layer) {
            auto layer_name{layer->name};
            string_utilities::capitalize(layer_name);
            return layer_name == name;
    });
    if (layer != layers.end())
        return layer->get();
    else
        return nullptr;
}

Image_Layer* Map::get_image_layer(int id) const {
    return dynamic_cast<Image_Layer*>(get_layer(id));
}

Image_Layer* Map::get_image_layer(const std::string& name) const {
    return dynamic_cast<Image_Layer*>(get_layer(name));
}

void Map::add_layer(Layer_Type layer_type) {
    std::shared_ptr<Layer> new_layer;
    switch (layer_type) {
    case Layer_Type::OBJECT:
        new_layer = std::make_shared<Object_Layer>();
        break;
    case Layer_Type::IMAGE:
        new_layer = std::make_shared<Image_Layer>();
        break;
    case Layer_Type::TILE:
        new_layer = std::make_shared<Tile_Layer>();
        break;
    default:
        return;
    }
    std::unordered_set<std::string> names;
    for (auto& layer : layers) {
        names.insert(layer->name);
    }
    new_layer->name = detail::generate_unique_name(names);
    layers.push_back(new_layer);
    if (layer_type == Layer_Type::OBJECT)
        object_layers.push_back(static_cast<Object_Layer*>(new_layer.get()));
}

void Map::delete_layer(std::string name) {
    string_utilities::capitalize(name);
    // Delete any matching object layer and its object
    for (auto layer = object_layers.begin(); layer != object_layers.end();)
    {
        auto layer_name{(*layer)->name};
        string_utilities::capitalize(layer_name);
        if (layer_name != name)
        {
            layer++;
        } else {
            for (auto& obj : (*layer)->objects)
            {
                erase_object_references(obj);
            }
            layer = object_layers.erase(layer);
        }
    }
    // Clear the collision object if necessary
    if (collision_layer) {
        auto collision_layer_name{collision_layer->name};
        string_utilities::capitalize(collision_layer_name);
        if (collision_layer_name == name) {
            collision_layer = nullptr;
        }
    }
    // Delete matching layers
    layers.erase(std::remove_if(layers.begin(), layers.end(),
        [&name](std::shared_ptr<Layer> layer) {
            auto layer_name{layer->name};
            string_utilities::capitalize(layer_name);
            return layer_name == name;
        }
    ), layers.end());
}

void Map::add_canvas(std::shared_ptr<Canvas> canvas) {
    canvases_sorted = false;
    canvas->set_priority(canvases.size());
    canvases.push_back(canvas);
}

const std::vector<std::weak_ptr<Canvas>>& Map::get_canvases() {
    if (!canvases_sorted) {
        canvases_sorted = true;
        std::sort(canvases.begin(), canvases.end(),
            [](std::weak_ptr<Canvas>& weak_a, std::weak_ptr<Canvas> weak_b) {
                auto a = weak_a.lock();
                auto b = weak_b.lock();
                if (a && b) {
                    return a->get_priority() < b->get_priority();
                } else if (a) {
                    return true;
                } else {
                    return false;
                }
            }
        );
    }
    return canvases;
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

void Map::save(std::string save_filename) {
    auto doc = std::make_unique< rapidxml::xml_document<>>();
    auto decl_node = xml_node(*doc, "", "", rapidxml::node_declaration);
    decl_node->append_attribute(xml_attribute(*doc, "version", "1.0"));
    decl_node->append_attribute(xml_attribute(*doc, "encoding", "UTF-8"));
    doc->append_node(decl_node);
    auto map_node = save(*doc);
    doc->append_node(map_node);
    std::ofstream out;
    out.open(save_filename, std::ios_base::out | std::ios_base::trunc);
    out << *doc;
}

rapidxml::xml_node<>* Map::save(rapidxml::xml_document<>& doc) {
    auto node = xml_node(doc, "map");
    node->append_attribute(xml_attribute(doc, "version", "1.0"));
    node->append_attribute(xml_attribute(doc, "orientation", "orthogonal"));
    node->append_attribute(xml_attribute(doc, "width", std::to_string(width)));
    node->append_attribute(xml_attribute(doc, "height", std::to_string(height)));
    node->append_attribute(xml_attribute(doc, "tilewidth", std::to_string(tile_width)));
    node->append_attribute(xml_attribute(doc, "tileheight", std::to_string(tile_height)));
    properties.save(doc, *node);
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
    LOGGER_I << "Loading map " << filename;
    auto doc = std::make_unique< rapidxml::xml_document<>>();
    char* content = doc->allocate_string(file_utilities::read_file(filename).c_str());
    doc->parse<0>(content);
    auto map_node = doc->first_node("map");
    if (!map_node)
        throw tmx_exception("Invalid TMX file. Missing map node");
    auto map = load(game, *map_node);
    map->filename = filename;
    file_utilities::normalize_slashes(map->filename);
    return map;
}

std::unique_ptr<Map> Map::load(Game& game, rapidxml::xml_node<>& node) {
    auto map_ptr = std::make_unique<Map>(game);

    if (node.first_attribute("orientation")->value() != std::string("orthogonal"))
        throw tmx_exception("Invalid map orientation, expected orthogonal");

    map_ptr->width = std::stoi(node.first_attribute("width")->value());
    map_ptr->height = std::stoi(node.first_attribute("height")->value());
    map_ptr->tile_width = std::stoi(node.first_attribute("tilewidth")->value());
    map_ptr->tile_height = std::stoi(node.first_attribute("tileheight")->value());

    // Map properties
    map_ptr->properties.read(node);

    // Background music
    if (map_ptr->properties.has_property("music")) {
        map_ptr->background_music = map_ptr->properties["music"];
    } else if (map_ptr->properties.has_property("music-script")) {
        auto si = game.get_current_scripting_interface();
        map_ptr->background_music = si->call<std::string>(map_ptr->properties["music-script"]);
    }

    // Startup scripts
    if (map_ptr->properties.has_property("scripts")) {
        auto filenames = string_utilities::split(map_ptr->properties["scripts"], ",");
        for (std::string filename : filenames) {
            string_utilities::trim(filename);
            map_ptr->start_scripts.push_back(filename);
        }
    }

    // Player position
    if (map_ptr->properties.has_property("player-position-x")) {
        map_ptr->starting_position.x = std::stof(map_ptr->properties["player-position-x"]);
    }
    else {
        map_ptr->starting_position.x = static_cast<float>(map_ptr->get_pixel_width() / 2);
    }
    if (map_ptr->properties.has_property("player-position-y")) {
        map_ptr->starting_position.y = std::stof(map_ptr->properties["player-position-y"]);
    }
    else {
        map_ptr->starting_position.y = static_cast<float>(map_ptr->get_pixel_height() / 2);
    }

    // Tilesets
    for (auto tileset_node = node.first_node("tileset");
            tileset_node; tileset_node = tileset_node->next_sibling("tileset")) {
        std::unique_ptr<Tileset> tileset_ptr;
        if (auto source_node = tileset_node->first_attribute("source")) {
            std::string source = source_node->value();
            tileset_ptr = Tileset::load(source);
            tileset_ptr->first_id = std::stoi(
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
                *game.get_camera()));
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

    // Set up chained outlining of objects
    for (auto& object : map_ptr->get_objects()) {
        auto target_id = object.second->get_outlined_object_id();
        auto target = map_ptr->get_object(target_id);
        if (!target) continue;

        target->set_outlining_object(object.second.get());
    }

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
    if (map.get_game().is_paused()) return;
    map.game.set_current_scripting_interface(map.scripting_interface.get());
    map.scripting_interface->update();
    for (auto& layer : map.layers) {
        if (auto updater = layer->updater.get())
            updater->update(map);
    }
}
