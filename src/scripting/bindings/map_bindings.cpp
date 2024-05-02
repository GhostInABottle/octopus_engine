#include "../../../include/map/layers/image_layer.hpp"
#include "../../../include/map/layers/object_layer.hpp"
#include "../../../include/map/map.hpp"
#include "../../../include/map/map_object.hpp"
#include "../../../include/scripting/script_bindings.hpp"
#include "../../../include/scripting/scripting_interface.hpp"
#include "../../../include/utility/file.hpp"
#include "../../../include/xd/vendor/sol/sol.hpp"
#include <string>

void bind_map_types(sol::state& lua) {
    // A data bag for defining custom properties on maps/canvases
    auto lua_object_type = lua.new_usertype<Lua_Object>("Lua_Object");
    lua_object_type[sol::meta_function::index] = &Lua_Object::get_lua_property;
    lua_object_type[sol::meta_function::new_index] = &Lua_Object::set_lua_property;

    // Current map / scene
    auto map_type = lua.new_usertype<Map>("Map");
    map_type["data"] = sol::property(&Map::get_lua_data);
    map_type["width"] = sol::property(&Map::get_width);
    map_type["height"] = sol::property(&Map::get_height);
    map_type["tile_width"] = sol::property(&Map::get_tile_width);
    map_type["tile_height"] = sol::property(&Map::get_tile_height);
    map_type["filename"] = sol::property(&Map::get_filename);
    map_type["filename_stem"] = sol::property([](const Map& map) {
        auto fs = file_utilities::game_data_filesystem();
        return fs->get_stem_component(map.get_filename());
    });
    map_type["name"] = sol::property(&Map::get_name);
    map_type["objects"] = sol::property([&](Map* map) {
        return sol::as_table(map->get_objects());
    });
    map_type["layer_count"] = sol::property(&Map::layer_count);
    map_type["object_count"] = sol::property(&Map::object_count);
    map_type["draw_outlines"] = sol::property(&Map::get_draw_outlines, &Map::set_draw_outlines);
    map_type["get_property"] = &Map::get_property;
    map_type["set_property"] = &Map::set_property;
    map_type["script_scheduler_paused"] = sol::property(&Map::is_script_scheduler_paused, &Map::set_script_scheduler_paused);
    map_type["get_object"] = sol::overload(
        sol::resolve<Map_Object* (int) const>(&Map::get_object),
        sol::resolve<Map_Object* (std::string) const>(&Map::get_object)
    );
    map_type["add_new_object"] = &Map::add_new_object;
    map_type["delete_object"] = (void (Map::*)(Map_Object*)) &Map::delete_object;
    map_type["get_layer"] = sol::overload(
        &Map::get_layer_by_id,
        &Map::get_layer_by_name
    );
    map_type["get_image_layer"] = sol::overload(
        &Map::get_image_layer_by_id,
        &Map::get_image_layer_by_name
    );
    map_type["get_object_layer"] = sol::overload(
        &Map::get_object_layer_by_id,
        &Map::get_object_layer_by_name
    );
    map_type["run_script"] = &Map::run_script;
    map_type["run_script_file"] = &Map::run_script_file;
    map_type["run_function"] = &Map::run_function;
    map_type["passable"] = [&](Map& map, const Map_Object& object, int dir) {
        auto c = map.passable(object, static_cast<Direction>(dir));
        return c.passable();
    };
    map_type["colliding_object"] = [&](Map& map, const Map_Object& object) -> Map_Object* {
        auto c = map.passable(object, object.get_direction(),
            Collision_Check_Type::OBJECT);
        if (c.type == Collision_Type::OBJECT)
            return c.other_object;
        else
            return nullptr;
    };
}
