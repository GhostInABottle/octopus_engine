#include "../../../include/canvas/image_canvas.hpp"
#include "../../../include/canvas/sprite_canvas.hpp"
#include "../../../include/canvas/text_canvas.hpp"
#include "../../../include/commands/command_result.hpp"
#include "../../../include/commands/move_canvas_command.hpp"
#include "../../../include/commands/show_pose_command.hpp"
#include "../../../include/commands/update_color_command.hpp"
#include "../../../include/commands/update_image_command.hpp"
#include "../../../include/commands/update_opacity_command.hpp"
#include "../../../include/game.hpp"
#include "../../../include/scripting/script_bindings.hpp"
#include "../../../include/scripting/scripting_interface.hpp"
#include "../../../include/utility/color.hpp"
#include "../../../include/xd/vendor/sol/sol.hpp"
#include <memory>
#include <optional>
#include <string>

namespace detail {
    template <typename T, typename CT>
    static void bind_get_child(sol::usertype<T> canvas_type, const char* method) {
        canvas_type[method] = sol::overload(
            [](Base_Canvas* cvs, std::size_t index) {
                return dynamic_cast<CT*>(cvs->get_child_by_index(index));
            },
            [](Base_Canvas* cvs, const std::string& name) {
                return dynamic_cast<CT*>(cvs->get_child_by_name(name));
            }
        );
    }

    template <typename T>
    static void bind_base_canvas(Game& game, sol::usertype<T> canvas_type) {
        canvas_type["data"] = sol::property(&T::get_lua_data);
        canvas_type["id"] = sol::property(&T::get_id);
        canvas_type["name"] = sol::property(&T::get_name, &T::set_name);
        canvas_type["priority"] = sol::property(&T::get_priority, &T::set_priority);
        canvas_type["position"] = sol::property(&T::get_position, &T::set_position);
        canvas_type["x"] = sol::property(&T::get_x, &T::set_x);
        canvas_type["y"] = sol::property(&T::get_y, &T::set_y);
        canvas_type["scissor_box"] = sol::property(&T::get_scissor_box, &T::set_scissor_box);
        canvas_type["camera_relative"] = sol::property(&T::is_camera_relative, &T::set_camera_relative);
        canvas_type["opacity"] = sol::property(&T::get_opacity, &T::set_opacity);
        canvas_type["child_count"] = sol::property(&T::get_child_count);
        canvas_type["has_background"] = sol::property(&T::has_background, &T::set_background_visible);
        canvas_type["background_color"] = sol::property(&T::get_background_color, &T::set_background_color);
        canvas_type["background_rect"] = sol::property(&T::get_background_rect, &T::set_background_rect);
        canvas_type["visible"] = sol::property(&T::is_visible, &T::set_visible);

        canvas_type["remove_child"] = &T::remove_child;

        bind_get_child<T, Text_Canvas>(canvas_type, "get_text_child");
        bind_get_child<T, Image_Canvas>(canvas_type, "get_image_child");
        bind_get_child<T, Sprite_Canvas>(canvas_type, "get_sprite_child");

        canvas_type["add_child_image"] = sol::overload(
            // with position
            [&](Base_Canvas& parent, const std::string& name, const std::string& filename, float x, float y) {
                return parent.add_child<Image_Canvas>(name, game, filename, xd::vec2{ x, y });
            },
            [&](Base_Canvas& parent, const std::string& name, const std::string& filename, xd::vec2 pos) {
                return parent.add_child<Image_Canvas>(name, game, filename, pos);
            },
            // with hex transparent color
            [&](Base_Canvas& parent, const std::string& name, const std::string& filename, float x, float y, const std::string& hex) {
                auto color = hex.empty() ? xd::vec4{ 0 } : hex_to_color(hex);
                return parent.add_child<Image_Canvas>(name, game, filename, xd::vec2{ x, y }, color);
            },
            [&](Base_Canvas& parent, const std::string& name, const std::string& filename, xd::vec2 pos, const std::string& hex) {
                auto color = hex.empty() ? xd::vec4{ 0 } : hex_to_color(hex);
                return parent.add_child<Image_Canvas>(name, game, filename, pos, color);
            },
            // with transparent color as vec4
            [&](Base_Canvas& parent, const std::string& name, const std::string& filename, float x, float y, xd::vec4 transparent) {
                return parent.add_child<Image_Canvas>(name, game, filename, xd::vec2{ x, y }, transparent);
            },
            [&](Base_Canvas& parent, const std::string& name, const std::string& filename, xd::vec2 pos, xd::vec4 transparent) {
                return parent.add_child<Image_Canvas>(name, game, filename, pos, transparent);
            }
        );

        canvas_type["add_child_sprite"] = sol::overload(
            // with position
            [&](Base_Canvas& parent, const std::string& name, const std::string& filename, float x, float y) {
                return parent.add_child<Sprite_Canvas>(name, game, filename, xd::vec2{ x, y });
            },
            [&](Base_Canvas& parent, const std::string& name, const std::string& filename, xd::vec2 pos) {
                return parent.add_child<Sprite_Canvas>(name, game, filename, pos);
            },
            // with pose name
            [&](Base_Canvas& parent, const std::string& name, const std::string& filename, float x, float y, const std::string& pose) {
                return parent.add_child<Sprite_Canvas>(name, game, filename, xd::vec2{ x, y }, pose);
            },
            [&](Base_Canvas& parent, const std::string& name, const std::string& filename, xd::vec2 pos, const std::string& pose) {
                return parent.add_child<Sprite_Canvas>(name, game, filename, pos, pose);
            }
        );

        // with text and position
        canvas_type["add_child_text"] = sol::overload(
            [&](Base_Canvas& parent, const std::string& name, float x, float y, const std::string& text) {
                return parent.add_child<Text_Canvas>(name, game, xd::vec2(x, y), text, true, std::nullopt, true);
            },
            [&](Base_Canvas& parent, const std::string& name, xd::vec2 pos, const std::string& text) {
                return parent.add_child<Text_Canvas>(name, game, pos, text, true, std::nullopt, true);
            }
        );

        canvas_type["show"] = [](T& canvas) {
            canvas.set_visible(true);
        };
        canvas_type["hide"] = [](T& canvas) {
            canvas.set_visible(false);
        };

        canvas_type["move"] = sol::overload(
            [&](T& canvas, xd::vec2 pos, long duration) {
                auto si = game.get_current_scripting_interface();
                return si->register_command<Move_Canvas_Command>(
                    game, canvas, pos, duration);
            },
            [&](T& canvas, float x, float y, long duration) {
                auto si = game.get_current_scripting_interface();
                return si->register_command<Move_Canvas_Command>(
                    game, canvas, xd::vec2(x, y), duration);
            }
        );

        canvas_type["update_opacity"] = [&](T& canvas, float opacity, long duration) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Update_Opacity_Command>(
                game, canvas, opacity, duration);
        };
        canvas_type["update_color"] = [&](T& canvas, xd::vec4 color, long duration) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Update_Color_Command>(
                game, canvas, color, duration);
        };
    }

    template <typename T>
    static void bind_base_image_canvas(Game& game, sol::usertype<T> canvas_type) {
        canvas_type["origin"] = sol::property(&T::get_origin, &T::set_origin);
        canvas_type["magnification"] = sol::property(&T::get_magnification, &T::set_magnification);
        canvas_type["angle"] = sol::property(&T::get_angle, &T::set_angle);
        canvas_type["filename"] = sol::property(&T::get_filename);
        canvas_type["color"] = sol::property(&T::get_color, &T::set_color);
        canvas_type["outline_color"] = sol::property(&T::get_outline_color, &T::set_outline_color);

        canvas_type["update"] = sol::overload(
            [&](T& canvas, float x, float y, float mag_x,
                float mag_y, float angle, float opacity, long duration) {
                    auto si = game.get_current_scripting_interface();
                    return si->register_command<Update_Image_Command>(
                        game, canvas, duration, xd::vec2(x, y),
                        xd::vec2(mag_x, mag_y), angle, opacity);
            },
            [&](T& canvas, xd::vec2 pos, xd::vec2 mag,
                float angle, float opacity, long duration) {
                    auto si = game.get_current_scripting_interface();
                    return si->register_command<Update_Image_Command>(
                        game, canvas, duration, pos, mag, angle, opacity);
            }
        );
        canvas_type["resize"] = sol::overload(
            [&](T& canvas, float mag, long duration) {
                auto si = game.get_current_scripting_interface();
                return si->register_command<Update_Image_Command>(
                    game, canvas, duration,
                    canvas.get_position(), xd::vec2(mag, mag),
                    canvas.get_angle(), canvas.get_opacity());
            },
            [&](T& canvas, float mag_x, float mag_y, long duration) {
                auto si = game.get_current_scripting_interface();
                return si->register_command<Update_Image_Command>(
                    game, canvas, duration,
                    canvas.get_position(), xd::vec2(mag_x, mag_y),
                    canvas.get_angle(), canvas.get_opacity());
            },
            [&](T& canvas, xd::vec2 mag, long duration) {
                auto si = game.get_current_scripting_interface();
                return si->register_command<Update_Image_Command>(
                    game, canvas, duration,
                    canvas.get_position(), mag,
                    canvas.get_angle(), canvas.get_opacity());
            }
        );
        canvas_type["rotate"] = [&](T& canvas, float angle, long duration) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Update_Image_Command>(
                game, canvas, duration, canvas.get_position(),
                canvas.get_magnification(), angle, canvas.get_opacity());
        };
    }
}

void bind_canvas_types(sol::state& lua, Game& game) {
    // A canvas for displaying a static image
    auto image_canvas_type = lua.new_usertype<Image_Canvas>("Image_Canvas",
        sol::call_constructor, sol::factories(
            // Canvas constructor (with position)
            [&](const std::string& filename, float x, float y) {
                auto canvas = std::make_shared<Image_Canvas>(game, filename, xd::vec2{ x, y });
                game.add_canvas(canvas);
                return canvas;
            },
            [&](const std::string& filename, xd::vec2 pos) {
                auto canvas = std::make_shared<Image_Canvas>(game, filename, pos);
                game.add_canvas(canvas);
                return canvas;
            },
            // Canvas constructor (with hex transparent color)
            [&](const std::string& filename, float x, float y, const std::string& color) {
                auto canvas = std::make_shared<Image_Canvas>(game, filename,
                    xd::vec2{ x, y }, hex_to_color(color));
                game.add_canvas(canvas);
                return canvas;
            },
            [&](const std::string& filename, xd::vec2 pos, const std::string& color) {
                auto canvas = std::make_shared<Image_Canvas>(game, filename,
                    pos, hex_to_color(color));
                game.add_canvas(canvas);
                return canvas;
            },
            // Canvas constructor (with transparent color as vec4)
            [&](const std::string& filename, float x, float y, xd::vec4 transparent) {
                auto canvas = std::make_shared<Image_Canvas>(game, filename,
                    xd::vec2{ x, y }, transparent);
                game.add_canvas(canvas);
                return canvas;
            },
            [&](const std::string& filename, xd::vec2 pos, xd::vec4 transparent) {
                auto canvas = std::make_shared<Image_Canvas>(game, filename, pos, transparent);
                game.add_canvas(canvas);
                return canvas;
            }
        ),
        sol::base_classes, sol::bases<Base_Canvas>()
    );

    detail::bind_base_canvas(game, image_canvas_type);
    detail::bind_base_image_canvas(game, image_canvas_type);
    image_canvas_type["set_image"] = [](Image_Canvas& canvas, const std::string& filename, std::optional<xd::vec4> ck) {
        canvas.set_image(filename, ck.value_or(xd::vec4{ 0 }));
    };
    image_canvas_type["width"] = sol::property(&Image_Canvas::get_width);
    image_canvas_type["height"] = sol::property(&Image_Canvas::get_height);

    // A canvas for displaying a sprite
    auto sprite_canvas_type = lua.new_usertype<Sprite_Canvas>("Sprite_Canvas",
        sol::call_constructor, sol::factories(
            // Canvas constructor (with position)
            [&](const std::string& filename, float x, float y) {
                auto canvas = std::make_shared<Sprite_Canvas>(game, filename, xd::vec2{ x, y });
                game.add_canvas(canvas);
                return canvas;
            },
            [&](const std::string& filename, xd::vec2 pos) {
                auto canvas = std::make_shared<Sprite_Canvas>(game, filename, pos);
                game.add_canvas(canvas);
                return canvas;
            },
            // Canvas constructor (with pose name)
            [&](const std::string& filename, float x, float y, const std::string& pose) {
                auto canvas = std::make_shared<Sprite_Canvas>(game, filename, xd::vec2{ x, y }, pose);
                game.add_canvas(canvas);
                return canvas;
            },
            [&](const std::string& filename, xd::vec2 pos, const std::string& pose) {
                auto canvas = std::make_shared<Sprite_Canvas>(game, filename, pos, pose);
                game.add_canvas(canvas);
                return canvas;
            }
        ),
        sol::base_classes, sol::bases<Base_Canvas>()
    );

    detail::bind_base_canvas(game, sprite_canvas_type);
    detail::bind_base_image_canvas(game, sprite_canvas_type);
    sprite_canvas_type["pose_name"] = sol::property(&Sprite_Canvas::get_pose_name);
    sprite_canvas_type["pose_state"] = sol::property(&Sprite_Canvas::get_pose_state);
    sprite_canvas_type["pose_direction"] = sol::property(&Sprite_Canvas::get_pose_direction);
    sprite_canvas_type["sprite"] = sol::property(
        &Sprite_Canvas::get_sprite_filename,
        [&](Sprite_Canvas* canvas, const std::string& filename) {
            canvas->set_sprite(game, filename);
        }
    );
    sprite_canvas_type["reset"] = &Sprite_Canvas::reset;
    sprite_canvas_type["set_sprite"] = [&](Sprite_Canvas* canvas, const std::string& filename, std::optional<std::string> pose) {
        canvas->set_sprite(game, filename, pose.value_or(""));
    };
    sprite_canvas_type["show_pose"] = [&](Sprite_Canvas* canvas, const std::string& pose_name, std::optional<std::string> state, std::optional<Direction> dir) {
        auto si = game.get_current_scripting_interface();
        auto holder_type = Show_Pose_Command::Holder_Type::CANVAS;
        auto id = canvas->get_id();
        auto root_parent = canvas->get_root_parent();
        auto parent_id = root_parent ? root_parent->get_id() : -1;
        Show_Pose_Command::Holder_Info holder_info{holder_type, id, parent_id};
        return si->register_command<Show_Pose_Command>(*game.get_map(),
            holder_info, pose_name, state.value_or(""), dir.value_or(Direction::NONE));
    };

    // A canvas for displaying text
    auto text_canvas_type = lua.new_usertype<Text_Canvas>("Text_Canvas",
        sol::call_constructor, sol::factories(
            // Canvas constructor (with text and position)
            [&](float x, float y, const std::string& text) {
                auto canvas = std::make_shared<Text_Canvas>(game, xd::vec2(x, y), text);
                game.add_canvas(canvas);
                return canvas;
            },
            [&](xd::vec2 pos, const std::string& text) {
                auto canvas = std::make_shared<Text_Canvas>(game, pos, text);
                game.add_canvas(canvas);
                return canvas;
            }
        ),
        sol::base_classes, sol::bases<Base_Canvas>()
    );

    detail::bind_base_canvas(game, text_canvas_type);
    text_canvas_type["text"] = sol::property(&Text_Canvas::get_text, &Text_Canvas::set_text);
    text_canvas_type["font_size"] = sol::property(&Text_Canvas::get_font_size, &Text_Canvas::set_font_size);
    text_canvas_type["color"] = sol::property(&Text_Canvas::get_color, &Text_Canvas::set_color);
    text_canvas_type["line_height"] = sol::property(&Text_Canvas::get_line_height, &Text_Canvas::set_line_height);
    text_canvas_type["outline_width"] = sol::property(&Text_Canvas::get_outline_width, &Text_Canvas::set_outline_width);
    text_canvas_type["outline_color"] = sol::property(&Text_Canvas::get_outline_color, &Text_Canvas::set_outline_color);
    text_canvas_type["shadow_offset"] = sol::property(&Text_Canvas::get_shadow_offset, &Text_Canvas::set_shadow_offset);
    text_canvas_type["shadow_color"] = sol::property(&Text_Canvas::get_shadow_color, &Text_Canvas::set_shadow_color);
    text_canvas_type["font_type"] = sol::property(&Text_Canvas::get_font_type, &Text_Canvas::set_font_type);
    text_canvas_type["permissive_tag_parsing"] = sol::property(&Text_Canvas::has_permissive_tag_parsing, &Text_Canvas::set_permissive_tag_parsing);

    text_canvas_type["reset_outline"] = &Text_Canvas::reset_outline;
    text_canvas_type["reset_shadow"] = &Text_Canvas::reset_shadow;
    text_canvas_type["reset_font_type"] = &Text_Canvas::reset_font_type;
    text_canvas_type["text_width"] = &Text_Canvas::get_text_width;
    text_canvas_type["set_font"] = &Text_Canvas::set_font;
    text_canvas_type["link_font"] = &Text_Canvas::link_font;
}
