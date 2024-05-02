#include "../../../include/commands/command_result.hpp"
#include "../../../include/commands/show_pose_command.hpp"
#include "../../../include/commands/update_color_command.hpp"
#include "../../../include/commands/update_layer_velocity_command.hpp"
#include "../../../include/commands/update_opacity_command.hpp"
#include "../../../include/game.hpp"
#include "../../../include/map/layers/image_layer.hpp"
#include "../../../include/map/layers/object_layer.hpp"
#include "../../../include/map/map.hpp"
#include "../../../include/map/map_object.hpp"
#include "../../../include/scripting/script_bindings.hpp"
#include "../../../include/scripting/scripting_interface.hpp"
#include "../../../include/xd/graphics/types.hpp"
#include "../../../include/xd/vendor/sol/sol.hpp"
#include <optional>
#include <string>

void bind_layer_types(sol::state& lua, Game& game) {
    // Map layer
    auto layer_type = lua.new_usertype<Layer>("Layer");
    layer_type["name"] = sol::readonly(&Layer::name);
    layer_type["visible"] = &Layer::visible;
    layer_type["opacity"] = &Layer::opacity;
    layer_type["get_property"] = &Layer::get_property;
    layer_type["set_property"] = &Layer::set_property;
    layer_type["update_opacity"] = [&](Layer* layer, float opacity, long duration) {
        auto si = game.get_current_scripting_interface();
        return si->register_command<Update_Opacity_Command>(
            game, *layer, opacity, duration);
    };

    // Image layer
    auto image_layer_type = lua.new_usertype<Image_Layer>("Image_Layer");
    image_layer_type["name"] = sol::readonly(&Image_Layer::name);
    image_layer_type["visible"] = &Image_Layer::visible;
    image_layer_type["opacity"] = &Image_Layer::opacity;
    image_layer_type["color"] = sol::property(&Image_Layer::get_color, &Image_Layer::set_color);
    image_layer_type["velocity"] = sol::property(&Image_Layer::get_velocity, &Image_Layer::set_velocity);
    image_layer_type["sprite"] = sol::property(
        &Image_Layer::get_sprite_filename,
        [&](Image_Layer* layer, const std::string& filename) {
            layer->set_sprite(game, filename);
        });
    image_layer_type["get_property"] = &Image_Layer::get_property;
    image_layer_type["set_property"] = &Image_Layer::set_property;
    image_layer_type["update_opacity"] = [&](Image_Layer* layer, float opacity, long duration) {
        auto si = game.get_current_scripting_interface();
        return si->register_command<Update_Opacity_Command>(
            game, *layer, opacity, duration);
    };
    image_layer_type["update_color"] = [&](Image_Layer* layer, xd::vec4 color, long duration) {
        auto si = game.get_current_scripting_interface();
        return si->register_command<Update_Color_Command>(
            game, *layer, color, duration);
    };
    image_layer_type["update_velocity"] = [&](Image_Layer* layer, xd::vec2 velocity, long duration) {
        auto si = game.get_current_scripting_interface();
        return si->register_command<Update_Layer_Velocity_Command>(
            game, *layer, velocity, duration);
    };
    image_layer_type["reset"] = &Image_Layer::reset;
    image_layer_type["set_sprite"] = [&](Image_Layer* layer, const std::string& filename, std::optional<std::string> pose) {
        layer->set_sprite(game, filename, pose.value_or(""));
    };
    image_layer_type["show_pose"] = [&](Image_Layer* layer, const std::string& pose_name, std::optional<std::string> state, std::optional<Direction> dir) {
        auto si = game.get_current_scripting_interface();
        auto holder_type = Show_Pose_Command::Holder_Type::LAYER;
        Show_Pose_Command::Holder_Info holder_info{ holder_type, layer->id };
        return si->register_command<Show_Pose_Command>(*game.get_map(),
            holder_info, pose_name, state.value_or(""), dir.value_or(Direction::NONE));
    };

    // Object layer
    auto object_layer_type = lua.new_usertype<Object_Layer>("Layer");
    object_layer_type["name"] = sol::readonly(&Object_Layer::name);
    object_layer_type["visible"] = &Object_Layer::visible;
    object_layer_type["opacity"] = &Object_Layer::opacity;
    object_layer_type["tint_color"] = sol::property(&Object_Layer::get_color, &Object_Layer::set_color);
    object_layer_type["objects"] = sol::property([&](Object_Layer* layer) {
        return sol::as_table(layer->objects);
    });
    object_layer_type["get_property"] = &Object_Layer::get_property;
    object_layer_type["set_property"] = &Object_Layer::set_property;
    object_layer_type["update_opacity"] = [&](Object_Layer* layer, float opacity, long duration) {
        auto si = game.get_current_scripting_interface();
        return si->register_command<Update_Opacity_Command>(
            game, *layer, opacity, duration);
    };
    object_layer_type["update_color"] = [&](Object_Layer* layer, xd::vec4 color, long duration) {
        auto si = game.get_current_scripting_interface();
        return si->register_command<Update_Color_Command>(
            game, *layer, color, duration);
    };
}
