#include "../../include/canvas/base_canvas.hpp"
#include "../../include/canvas/canvas_renderer.hpp"
#include "../../include/canvas/canvas_updater.hpp"
#include "../../include/configurations.hpp"
#include "../../include/exceptions.hpp"
#include "../../include/game.hpp"
#include "../../include/log.hpp"
#include "../../include/map/layers/image_layer.hpp"
#include "../../include/map/layers/layer_renderer.hpp"
#include "../../include/map/layers/layer_updater.hpp"
#include "../../include/map/layers/object_layer.hpp"
#include "../../include/map/layers/tile_layer.hpp"
#include "../../include/map/map.hpp"
#include "../../include/map/map_object.hpp"
#include "../../include/map/tileset.hpp"
#include "../../include/scripting/scripting_interface.hpp"
#include "../../include/utility/direction.hpp"
#include "../../include/utility/file.hpp"
#include "../../include/utility/string.hpp"
#include "../../include/utility/xml.hpp"
#include "../../include/vendor/rapidxml_print.hpp"
#include "../../include/xd/vendor/sol/sol.hpp"
#include <algorithm>
#include <fstream>
#include <limits>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {
    static std::string generate_unique_name(std::unordered_set<std::string> names,
            std::string base_name = "UNTITLED") {
        int i = 1;
        string_utilities::capitalize(base_name);
        std::string name = base_name;
        while (names.find(name) != names.end()) {
            name = base_name + std::to_string(i++);
        }
        return name;
    }

    static inline bool circle_intersects(const Map_Object& obj1, const Map_Object& obj2,
            const xd::vec2& obj1_pos, float proximity_distance = 0.0f) {
        auto obj1_circle = obj1.get_bounding_circle();
        auto obj2_circle = obj2.get_positioned_bounding_circle();
        if (!obj1_circle && !obj2_circle) {
            return false;
        }

        if (obj1_circle) {
            obj1_circle->extend(proximity_distance);
            obj1_circle->move(obj1_pos);
        }

        if (obj1_circle && obj2_circle) {
            return obj1_circle->intersects(obj2_circle.value());
        }

        if (obj1_circle) {
            auto boj2_box = obj2.get_positioned_bounding_box();
            return obj1_circle->intersects(boj2_box);
        }

        xd::rect obj1_box{ obj1.get_bounding_box() };
        obj1_box.move(obj1_pos);
        return obj2_circle->intersects(obj1_box.extend(proximity_distance));
    }

    static inline bool check_intersection(const Map_Object& obj1, const Map_Object& obj2,
            const xd::rect& obj1_box, const xd::vec2& obj1_pos, float proximity_distance = 0.0f) {
        auto obj2_box = obj2.get_positioned_bounding_box();
        auto intersects = obj1_box.intersects(obj2_box);

        auto check_circle_intersection = intersects
            && (obj1.get_bounding_circle() || obj2.get_bounding_circle());
        if (check_circle_intersection) {
            intersects = circle_intersects(obj1, obj2, obj1_pos, proximity_distance);
        }

        return intersects;
    }

    static inline std::tuple<int, float> distance_and_dot(const xd::rect& obj1_box, const xd::rect& obj2_box, Direction dir) {
        auto obj1_center = obj1_box.center();
        auto obj2_center = obj2_box.center();
        auto obj1_dir_vector = direction_to_vector(dir);
        auto facing_dir_vector = direction_to_vector(facing_direction(obj1_center, obj2_center, true));
        return { static_cast<int>(glm::distance(obj1_center, obj2_center)),
            glm::dot(obj1_dir_vector, facing_dir_vector) };
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
        background_music_volume(1.0f),
        background_ambient_volume(1.0f),
        draw_object_outlines(true),
        needs_redraw(true),
        objects_moved(true),
        canvases_sorted(false),
        last_typewriter_slot(100) {
    Base_Canvas::reset_last_child_id();
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
        auto scripts_folder = game.get_scripts_directory();
        scripting_interface->schedule_file(scripts_folder + script_or_filename, "MAP");
    } else {
        scripting_interface->schedule_code(script_or_filename, "MAP");
    }
    game.set_current_scripting_interface(old_interface);
}

void Map::run_function(const sol::protected_function& function) {
    auto old_interface = game.get_current_scripting_interface();
    game.set_current_scripting_interface(scripting_interface.get());
    scripting_interface->schedule_function(function, "MAP");
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

bool Map::is_script_scheduler_paused() const {
    return scripting_interface->get_scheduler().paused();
}

void Map::set_script_scheduler_paused(bool paused) {
    auto& scheduler = scripting_interface->get_scheduler();
    if (paused)
        scheduler.pause();
    else
        scheduler.resume();
}

Collision_Record Map::passable(Collision_Check_Options options) const {
    Collision_Record result;

    auto& object = options.object;
    if (object.initiates_passthrough())
        return result;

    const auto& bounding_box = object.get_bounding_box();
    if (bounding_box.w <= 0.0f || bounding_box.h <= 0.0f)
        return result;

    auto change = direction_to_vector(options.direction) * options.speed;
    auto new_pos = options.position + change;
    xd::rect this_box{ bounding_box.position() + new_pos, bounding_box.size() };

    bool check_tile_collision = options.check_type & Collision_Check_Type::TILE;
    int minimum_distance = std::numeric_limits<int>::max();

    // Check object collisions
    if (options.check_type & Collision_Check_Type::OBJECT) {
        for (auto& object_pair : objects) {
            auto other_id = object_pair.first;
            auto other_object = object_pair.second.get();

            // Skip invisible objects and self-intersection
            if (!other_object->is_visible() || other_id == object.get_id()) continue;

            // Skip objects with no bounding box
            const auto& box = other_object->get_bounding_box();
            if (box.w <= 0.0f || box.h <= 0.0f) continue;

            auto other_box = other_object->get_positioned_bounding_box();
            auto intersects = check_intersection(object, *other_object, this_box, new_pos);

            // Special case for skipping tile collision detection
            const auto passthrough = other_object->receives_passthrough();
            if (other_object->overrides_tile_collision() && passthrough && intersects)
                check_tile_collision = false;

            // Areas are passthrough objects with a script
            const auto has_script = other_object->has_any_script();
            const auto is_area = passthrough && has_script;

            // Check proximate objects
            bool check_nearby = !intersects
                && !passthrough
                && has_script
                && !result.other_object
                && other_object->proximity_distance() != 0;
            if (check_nearby) {
                auto int_proximity_distance = other_object->proximity_distance();
                auto proximity_distance = static_cast<float>(int_proximity_distance == -1
                    ? options.proximity_distance
                    : int_proximity_distance);
                const auto nearby_box = this_box.extend(proximity_distance);
                auto nearby_intersects = check_intersection(object, *other_object,
                    nearby_box, new_pos, proximity_distance);

                auto [distance, dot] = nearby_intersects
                    ? distance_and_dot(nearby_box, other_box, options.direction)
                    : std::make_tuple(-1, 0.0f);
                if (nearby_intersects && distance < minimum_distance && dot > 0) {
                    result.proximate_object = other_object;
                    minimum_distance = distance;
                }
            }

            // Skip non-intersecting or passthrough objects (except areas)
            bool skip = !intersects || (!is_area && passthrough);
            if (skip) continue;

            if (is_area) {
                if (result.type == Collision_Type::NONE) {
                    result.type = Collision_Type::AREA;
                }
                result.other_area = other_object;
                continue;
            }

            result.type = Collision_Type::OBJECT;
            // Prefer objects with scripts
            if (!result.other_object || has_script) {
                result.other_object = other_object;
                result.proximate_object = other_object;
            }
            check_tile_collision = false;
        }
    }

    // Check tile collisions (unless an object overrides collision)
    if (!check_tile_collision) return result;

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
    const auto& tiles = collision_layer->get_tiles();

    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            // Maximum map bounds check (taking collision box into account)
            if (x >= width || y >= height) {
                result.type = Collision_Type::TILE;
                return result;
            }
            // Check if tile is blocking
            if (collision_layer && collision_tileset) {
                const int tile = tiles[x + y * width] -
                    collision_tileset->first_id;
                if (tile >= 2) {
                    result.type = Collision_Type::TILE;
                    return result;
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
    const auto& tiles = collision_layer->get_tiles();
    return tiles[tile_index] - collision_tileset->first_id <= 1;
}

int Map::object_count() const noexcept {
    return objects.size();
}

Map_Object* Map::add_object(const std::shared_ptr<Map_Object>& object, Object_Layer* layer) {
    // Update ID counter for new objects
    int id = object->get_id();
    if (id == -1) {
        id = next_object_id;
        object->set_id(id);
    }
    if (id >= next_object_id) {
        next_object_id = id + 1;
    }

    auto name = object->get_name();
    if (name.empty()) {
        name = "UNTITLED" + std::to_string(id);
        object->set_name(name);
    }
    string_utilities::capitalize(name);

    if (get_object(id)) {
        throw std::runtime_error("Trying to add object with existing ID " + std::to_string(id));
    }
    if (get_object(name)) {
        throw std::runtime_error("Trying to add object with existing name " + name);
    }
    object_name_to_id[name] = id;
    objects[id] = object;
    object->set_name(name);

    // If layer isn't specified try getting a layer named "objects",
    // if none is found use the 'middle' object layer
    if (!layer) {
        layer = static_cast<Object_Layer*>(get_layer_by_name("objects"));
        if (!layer) {
            int index = static_cast<int>(std::floor(object_layers.size() / 2.0));
            layer = object_layers[index];
        }
    }
    move_object_to_layer(object.get(), layer);

    return object.get();
}

Map_Object* Map::add_new_object(std::optional<std::string> name, std::optional<std::string> sprite_file,
        std::optional<xd::vec2> pos, std::optional<Direction> dir, std::optional<Object_Layer*> layer) {
    return add_object(std::make_shared<Map_Object>(game, name.value_or(""), sprite_file.value_or(""),
        pos.value_or(xd::vec2{}), dir.value_or(Direction::DOWN)), layer.value_or(nullptr));
}

void Map::move_object_to_layer(Map_Object* object, Object_Layer* layer) {
    auto old_layer = object->get_layer();
    if (layer == old_layer) return;

    if (old_layer) {
        auto& old_layer_objects = old_layer->get_objects();
        auto removed_start = std::remove(old_layer_objects.begin(),
            old_layer_objects.end(), object);
        old_layer_objects.erase(removed_start, old_layer_objects.end());
    }

    object->set_layer(layer);

    auto& layer_objects = layer->get_objects();
    layer_objects.push_back(object);
}

Map_Object* Map::get_object(std::string name) const {
    string_utilities::capitalize(name);
    if (object_name_to_id.find(name) != object_name_to_id.end()) {
        return get_object(object_name_to_id.at(name));
    }
    return nullptr;
}

Map_Object* Map::get_object(int id) const {
    auto obj = objects.find(id);
    return obj != objects.end() ? obj->second.get() : nullptr;
}

void Map::delete_object(const std::string& name) {
    delete_object(get_object(name));
}

void Map::delete_object(int id) {
    delete_object(get_object(id));
}

void Map::delete_object(Map_Object* object) {
    if (!object) return;

    auto& layer_objects = object->get_layer()->get_objects();
    layer_objects.erase(
        std::remove(layer_objects.begin(), layer_objects.end(), object),
        layer_objects.end()
    );
    erase_object_references(object);
}

void Map::erase_object_references(const Map_Object* object) {
    auto player = game.get_player();
    if (player) {
        if (object == player->get_triggered_object()) {
            player->set_triggered_object(nullptr);
        }
        if (object == player->get_collision_object()) {
            player->set_collision_object(nullptr);
        }
        if (object == player->get_collision_area()) {
            player->set_collision_area(nullptr);
        }
        if (object == player->get_proximate_object()) {
            player->set_proximate_object(nullptr);
        }
        if (object == player->get_outlining_object()) {
            player->set_outlining_object(nullptr);
        }
    }

    auto name = object->get_name();
    string_utilities::capitalize(name);
    object_name_to_id.erase(name);

    objects.erase(object->get_id());
}

int Map::layer_count() const {
    return layers.size();
}

Layer* Map::get_layer_by_index(int index) const {
    if (index >= 1 && index <= layer_count())
        return layers[index - 1].get();
    else
        return nullptr;
}

Layer* Map::get_layer_by_id(int id) const {
    auto result = layers_by_id.find(id);

    if (result != layers_by_id.end())
        return result->second;
    else
        return nullptr;
}

Layer* Map::get_layer_by_name(std::string name) const {
    string_utilities::capitalize(name);
    auto layer = std::find_if(layers.begin(), layers.end(),
        [&name](std::shared_ptr<Layer> layer) {
            auto layer_name{layer->get_name()};
            string_utilities::capitalize(layer_name);
            return layer_name == name;
    });
    if (layer != layers.end())
        return layer->get();
    else
        return nullptr;
}

Image_Layer* Map::get_image_layer_by_index(int index) const {
    return dynamic_cast<Image_Layer*>(get_layer_by_index(index));
}

Image_Layer* Map::get_image_layer_by_id(int id) const {
    return dynamic_cast<Image_Layer*>(get_layer_by_id(id));
}

Image_Layer* Map::get_image_layer_by_name(const std::string& name) const {
    return dynamic_cast<Image_Layer*>(get_layer_by_name(name));
}

Object_Layer* Map::get_object_layer_by_index(int index) const {
    return dynamic_cast<Object_Layer*>(get_layer_by_index(index));
}

Object_Layer* Map::get_object_layer_by_id(int id) const {
    return dynamic_cast<Object_Layer*>(get_layer_by_id(id));
}

Object_Layer* Map::get_object_layer_by_name(const std::string& name) const {
    return dynamic_cast<Object_Layer*>(get_layer_by_name(name));
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
        names.insert(layer->get_name());
    }
    new_layer->set_name(generate_unique_name(names));

    add_layer(new_layer);

    if (layer_type == Layer_Type::OBJECT) {
        object_layers.push_back(static_cast<Object_Layer*>(new_layer.get()));
    }
}

void Map::add_layer(std::shared_ptr<Layer> layer) {
    layers.push_back(layer);
    layers_by_id[layer->get_id()] = layer.get();
}

void Map::delete_layer(std::string name) {
    string_utilities::capitalize(name);
    // Delete any matching object layer and its object
    for (auto layer = object_layers.begin(); layer != object_layers.end();) {
        auto layer_name{(*layer)->get_name()};
        string_utilities::capitalize(layer_name);
        if (layer_name != name) {
            layer++;
        } else if (object_layers.size() > 1) {
            const auto& layer_objects = (*layer)->get_objects();
            for (auto& obj : layer_objects) {
                erase_object_references(obj);
            }
            layer = object_layers.erase(layer);
        } else {
            throw tmx_exception("Must have at least one object layer in map");
        }
    }
    // Clear the collision object if necessary
    if (collision_layer) {
        auto collision_layer_name{collision_layer->get_name()};
        string_utilities::capitalize(collision_layer_name);
        if (collision_layer_name == name) {
            collision_layer = nullptr;
        }
    }
    // Delete matching layers
    auto& layer_lookup = layers_by_id;
    auto erase_start = std::remove_if(layers.begin(), layers.end(),
        [&name, &layer_lookup](std::shared_ptr<Layer> layer) {
            auto layer_name{layer->get_name()};
            string_utilities::capitalize(layer_name);
            if (layer_name != name) return false;

            layer_lookup.erase(layer->get_id());
            return true;
        }
    );
    layers.erase(erase_start, std::end(layers));
}

void Map::add_canvas(std::shared_ptr<Base_Canvas> canvas) {
    if (canvases_sorted) {
        // Clean up expired references, but only once when multiple canvases
        // are being inserted at once
        erase_canvases();
    }
    canvases_sorted = false;
    canvas->set_priority(canvases.size());
    auto id = canvas->get_id();
    auto ref = Canvas_Ref{ id, canvas };
    canvases.push_back(ref);
    canvases_by_id[id] = canvas;
}

void Map::erase_canvases() {
    auto& canvas_lookup = canvases_by_id;
    auto erase_start = std::remove_if(std::begin(canvases), std::end(canvases),
        [&canvas_lookup](const Canvas_Ref& ref) {
            auto canvas = ref.ptr.lock();
            if (canvas) return false;

            canvas_lookup.erase(ref.id);
            return true;
        });
    canvases.erase(erase_start, std::end(canvases));
}

const std::vector<Map::Canvas_Ref>& Map::get_canvases() {
    if (!canvases_sorted) {
        canvases_sorted = true;
        std::sort(canvases.begin(), canvases.end(),
            [](Canvas_Ref& canvas_a, Canvas_Ref& canvas_b) {
                auto a = canvas_a.ptr.lock();
                auto b = canvas_b.ptr.lock();
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

Base_Canvas* Map::get_canvas(int id) const {
    auto result = canvases_by_id.find(id);

    if (result != canvases_by_id.end() && result->second.lock())
        return result->second.lock().get();
    else
        return nullptr;
}

void Map::resize(xd::ivec2 map_size, xd::ivec2 tile_size) {
    auto same_size = map_size == xd::ivec2(width, height)
        && tile_size == xd::ivec2(tile_width, tile_height);
    if (same_size) return;

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

    auto fs = file_utilities::disk_filesystem();
    auto out = fs->open_ofstream(save_filename, std::ios_base::out | std::ios_base::trunc);
    if (!out || !*out) return;

    rapidxml::print_stream(*out, *doc, rapidxml::print_space_indenting);
}

rapidxml::xml_node<>* Map::save(rapidxml::xml_document<>& doc) {
    auto node = xml_node(doc, "map");
    node->append_attribute(xml_attribute(doc, "version", "1.0"));
    node->append_attribute(xml_attribute(doc, "orientation", "orthogonal"));
    node->append_attribute(xml_attribute(doc, "renderorder", "right-down"));
    node->append_attribute(xml_attribute(doc, "width", std::to_string(width)));
    node->append_attribute(xml_attribute(doc, "height", std::to_string(height)));
    node->append_attribute(xml_attribute(doc, "tilewidth", std::to_string(tile_width)));
    node->append_attribute(xml_attribute(doc, "tileheight", std::to_string(tile_height)));
    node->append_attribute(xml_attribute(doc, "nextobjectid", std::to_string(next_object_id)));
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
    auto fs = file_utilities::game_data_filesystem();
    char* content = doc->allocate_string(fs->read_file(filename).c_str());
    doc->parse<0>(content);

    auto map_node = doc->first_node("map");
    if (!map_node)
        throw tmx_exception("Invalid TMX file. Missing map node");

    auto map = load(game, *map_node);

    map->filename = filename;
    string_utilities::normalize_slashes(map->filename);

    return map;
}

std::unique_ptr<Map> Map::load(Game& game, rapidxml::xml_node<>& node) {
    auto map_ptr = std::make_unique<Map>(game);

    if (node.first_attribute("orientation")->value() != std::string("orthogonal"))
        throw tmx_exception("Invalid map orientation, expected orthogonal");

    if (auto width_node = node.first_attribute("width"))
        map_ptr->width = std::stoi(width_node->value());
    else
        throw tmx_exception("Map width is missing");

    if (auto height_node = node.first_attribute("height"))
        map_ptr->height = std::stoi(height_node->value());
    else
        throw tmx_exception("Map height is missing");

    if (auto tile_width_node = node.first_attribute("tilewidth"))
        map_ptr->tile_width = std::stoi(tile_width_node->value());
    else
        throw tmx_exception("Map tile width is missing");

    if (auto tile_height_node = node.first_attribute("tileheight"))
        map_ptr->tile_height = std::stoi(tile_height_node->value());
    else
        throw tmx_exception("Map tile height is missing");

    // Map properties
    map_ptr->properties.read(node);

    // Background music and ambient
    if (map_ptr->properties.contains("music")) {
        map_ptr->background_music_filename = map_ptr->properties["music"];
    }
    if (map_ptr->properties.contains("music-volume")) {
        map_ptr->background_music_volume = std::stof(map_ptr->properties["music-volume"]);
    }

    if (map_ptr->properties.contains("ambient")) {
        map_ptr->background_ambient_filename = map_ptr->properties["ambient"];
    }
    if (map_ptr->properties.contains("ambient-volume")) {
        map_ptr->background_ambient_volume = std::stof(map_ptr->properties["ambient-volume"]);
    }

    if (map_ptr->properties.contains("music-script")) {
        if (!map_ptr->background_music_filename.empty())
            throw tmx_exception("Tried to set a map music script, but music was already specified as: " + map_ptr->background_music_filename);

        auto si = game.get_current_scripting_interface();
        map_ptr->background_music_filename = si->call<std::string>(map_ptr->properties["music-script"]);
    }

    // Startup scripts
    if (map_ptr->properties.contains("scripts")) {
        auto filenames = string_utilities::split(map_ptr->properties["scripts"], ",");
        for (std::string filename : filenames) {
            string_utilities::trim(filename);
            map_ptr->start_scripts.push_back(filename);
        }
    }

    // Player position
    if (map_ptr->properties.contains("player-position-x")) {
        map_ptr->starting_position.x = std::stof(map_ptr->properties["player-position-x"]);
    } else {
        map_ptr->starting_position.x = static_cast<float>(map_ptr->get_pixel_width() / 2);
    }

    if (map_ptr->properties.contains("player-position-y")) {
        map_ptr->starting_position.y = std::stof(map_ptr->properties["player-position-y"]);
    } else {
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
            if (layer->get_name() == "collision") {
                layer->set_visible(false);
                map_ptr->collision_layer = static_cast<Tile_Layer*>(layer.get());
            }
        } else if (node_name == "imagelayer") {
            layer = std::shared_ptr<Layer>(Image_Layer::load(*layer_node, game,
                *game.get_camera()));
        } else if (node_name == "objectgroup") {
            layer = std::shared_ptr<Layer>(Object_Layer::load(*layer_node, game, *game.get_camera(), *map_ptr));
            map_ptr->object_layers.push_back(static_cast<Object_Layer*>(layer.get()));
        }

        if (layer.get()) {
            map_ptr->add_layer(layer);
        }

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
        if (auto renderer = layer->get_renderer()) {
            if (map.needs_redraw) {
                renderer->redraw();
            }

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
        if (auto updater = layer->get_updater()) {
            updater->update(map);
        }
    }
}
