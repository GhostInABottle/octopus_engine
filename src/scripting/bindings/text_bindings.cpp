#include "../../../include/commands/command_result.hpp"
#include "../../../include/commands/show_text_command.hpp"
#include "../../../include/game.hpp"
#include "../../../include/map/map_object.hpp"
#include "../../../include/scripting/script_bindings.hpp"
#include "../../../include/scripting/scripting_interface.hpp"
#include "../../../include/text_parser.hpp"
#include "../../../include/xd/glm.hpp"
#include "../../../include/xd/vendor/sol/sol.hpp"
#include <string>
#include <utility>

namespace detail {
    static Text_Options options_from_table(const sol::table& table) {
        Text_Options options;
        if (table["text"].valid()) {
            options.set_text(table["text"]);
        }

        if (table["choices"].valid()) {
            sol::table choices_table = table["choices"];
            std::vector<std::string> choices;
            for (auto& kv : choices_table) {
                choices.push_back(kv.second.as<std::string>());
            }
            options.set_choices(choices);
        }

        if (table["object"].valid()) {
            options.set_object(table["object"]);
        }

        if (table["position"].valid()) {
            options.set_position(table["position"]);
        }

        if (table["position_type"].valid()) {
            options.set_position_type(table["position_type"]);
        }

        if (table["duration"].valid()) {
            options.set_duration(table["duration"]);
        }

        if (table["centered"].valid()) {
            options.set_centered(table["centered"]);
        }

        if (table["show_dashes"].valid()) {
            options.set_show_dashes(table["show_dashes"]);
        }

        if (table["cancelable"].valid()) {
            options.set_cancelable(table["cancelable"]);
        }

        if (table["choice_indent"].valid()) {
            options.set_choice_indent(table["choice_indent"]);
        }

        if (table["canvas_priority"].valid()) {
            options.set_canvas_priority(table["canvas_priority"]);
        }

        if (table["fade_in_duration"].valid()) {
            options.set_fade_in_duration(table["fade_in_duration"]);
        }

        if (table["fade_out_duration"].valid()) {
            options.set_fade_out_duration(table["fade_out_duration"]);
        }

        if (table["background_visible"].valid()) {
            options.set_background_visible(table["background_visible"]);
        }

        if (table["background_color"].valid()) {
            options.set_background_color(table["background_color"]);
        }

        if (table["typewriter_on"].valid()) {
            options.set_typewriter_on(table["typewriter_on"]);
        }

        if (table["typewriter_delay"].valid()) {
            options.set_typewriter_delay(table["typewriter_delay"]);
        }

        if (table["typewriter_sound"].valid()) {
            options.set_typewriter_sound(table["typewriter_sound"]);
        }

        if (table["typewriter_sound_volume"].valid()) {
            options.set_typewriter_sound_volume(table["typewriter_sound_volume"]);
        }

        if (table["typewriter_sound_pitch"].valid()) {
            options.set_typewriter_sound_pitch(table["typewriter_sound_pitch"]);
        }

        if (table["typewriter_sound_max_pitch"].valid()) {
            options.set_typewriter_sound_max_pitch(table["typewriter_sound_max_pitch"]);
        }

        if (table["typewriter_skippable"].valid()) {
            options.set_typewriter_skippable(table["typewriter_skippable"]);
        }

        return options;
    }
}

void bind_text_types(sol::state& lua, Game& game) {
    // Text positioning
    lua.new_enum<Text_Position_Type>("Text_Position_Type",
        {
            {"none", Text_Position_Type::NONE},
            {"exact_x", Text_Position_Type::EXACT_X},
            {"centered_x", Text_Position_Type::CENTERED_X},
            {"exact_y", Text_Position_Type::EXACT_Y},
            {"bottom_y", Text_Position_Type::BOTTOM_Y},
            {"camera_relative", Text_Position_Type::CAMERA_RELATIVE},
            {"always_visible", Text_Position_Type::ALWAYS_VISIBLE}
        });

    // Options for displaying text
    auto options_type = lua.new_usertype<Text_Options>("Text_Options",
        sol::call_constructor, sol::constructors<Text_Options(), Text_Options(Map_Object*), Text_Options(xd::vec2)>());

    options_type["text"] = sol::readonly(&Text_Options::text);
    options_type["choices"] = sol::readonly(&Text_Options::choices);
    options_type["object"] = sol::readonly(&Text_Options::object);
    options_type["position"] = sol::readonly(&Text_Options::position);
    options_type["position_type"] = sol::readonly(&Text_Options::position_type);
    options_type["duration"] = sol::readonly(&Text_Options::duration);
    options_type["centered"] = sol::readonly(&Text_Options::centered);
    options_type["show_dashes"] = sol::readonly(&Text_Options::show_dashes);
    options_type["cancelable"] = sol::readonly(&Text_Options::cancelable);
    options_type["choice_indent"] = sol::readonly(&Text_Options::choice_indent);
    options_type["canvas_priority"] = sol::readonly(&Text_Options::canvas_priority);
    options_type["fade_in_duration"] = sol::readonly(&Text_Options::fade_in_duration);
    options_type["fade_out_duration"] = sol::readonly(&Text_Options::fade_out_duration);
    options_type["background_visible"] = sol::readonly(&Text_Options::background_visible);
    options_type["background_color"] = sol::readonly(&Text_Options::background_color);
    options_type["typewriter_on"] = sol::readonly(&Text_Options::typewriter_on);
    options_type["typewriter_delay"] = sol::readonly(&Text_Options::typewriter_delay);
    options_type["typewriter_sound"] = sol::readonly(&Text_Options::typewriter_sound);
    options_type["typewriter_skippable"] = sol::readonly(&Text_Options::typewriter_skippable);

    options_type["set_text"] = &Text_Options::set_text;
    options_type["set_object"] = &Text_Options::set_object;
    options_type["set_position"] = &Text_Options::set_position;
    options_type["set_duration"] = &Text_Options::set_duration;
    options_type["set_centered"] = &Text_Options::set_centered;
    options_type["set_show_dashes"] = &Text_Options::set_show_dashes;
    options_type["set_cancelable"] = &Text_Options::set_cancelable;
    options_type["set_choice_indent"] = &Text_Options::set_choice_indent;
    options_type["set_canvas_priority"] = &Text_Options::set_canvas_priority;
    options_type["set_fade_in_duration"] = &Text_Options::set_fade_in_duration;
    options_type["set_fade_out_duration"] = &Text_Options::set_fade_out_duration;
    options_type["set_background_visible"] = &Text_Options::set_background_visible;
    options_type["set_background_color"] = &Text_Options::set_background_color;
    options_type["set_typewriter_on"] = &Text_Options::set_typewriter_on;
    options_type["set_typewriter_delay"] = &Text_Options::set_typewriter_delay;
    options_type["set_typewriter_sound"] = &Text_Options::set_typewriter_sound;
    options_type["set_typewriter_skippable"] = &Text_Options::set_typewriter_skippable;
    options_type["set_choices"] = [&](Text_Options& options, const sol::table& table) -> Text_Options& {
        std::vector<std::string> choices;
        for (auto& kv : table) {
            choices.push_back(kv.second.as<std::string>());
        }
        return options.set_choices(choices);
    };
    options_type["set_position_type"] = &Text_Options::set_position_type;

    // Show text
    lua["text"] = sol::overload(
        [&](const Text_Options& options) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Show_Text_Command>(game, options);
        },
        [&](const sol::table& table) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Show_Text_Command>(game, detail::options_from_table(table));
        },
        [&](Map_Object& obj, const std::string& text) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Show_Text_Command>(
                game, Text_Options{ &obj }.set_text(text));
        },
        [&](xd::vec2& position, const std::string& text) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Show_Text_Command>(
                game, Text_Options{ position }.set_text(text));
        },
        [&](Map_Object& obj, const std::string& text, long duration) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Show_Text_Command>(
                game, Text_Options{ &obj }.set_text(text).set_duration(duration));
        },
        [&](xd::vec2& position, const std::string& text, long duration) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Show_Text_Command>(
                game, Text_Options{ position }.set_text(text).set_duration(duration));
        }
    );

    lua["centered_text"] = sol::overload(
        [&](float y, const std::string& text) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Show_Text_Command>(
                game, Text_Options{ xd::vec2{0.0f, y} }.set_text(text).set_centered(true));
        },
        [&](float y, const std::string& text, long duration) {
            auto si = game.get_current_scripting_interface();
            return si->register_command<Show_Text_Command>(
                game, Text_Options{ xd::vec2{0.0f, y} }.set_text(text).set_duration(duration).set_centered(true));
        }
    );

    // Show some text followed by a list of choices
    auto set_options = [](Text_Options& options, const std::vector<std::string>& choices,
        const std::string& text, std::optional<bool> cancelable) {
            options.set_choices(choices).set_text(text).set_cancelable(cancelable.value_or(false));
    };

    lua["choices"] = sol::overload(
        [&](const Text_Options& options) {
            auto si = game.get_current_scripting_interface();
            return si->register_choice_command<Show_Text_Command>(game, options);
        },
        [&](const sol::table& table) {
            auto si = game.get_current_scripting_interface();
            return si->register_choice_command<Show_Text_Command>(game, detail::options_from_table(table));
        },
        [&](Map_Object& obj, const std::string& text, const sol::table& table, std::optional<bool> cancelable) {
            std::vector<std::string> choices;
            for (auto& kv : table) {
                choices.push_back(kv.second.as<std::string>());
            }
            Text_Options options{ &obj };
            set_options(options, choices, text, cancelable);
            auto si = game.get_current_scripting_interface();
            return si->register_choice_command<Show_Text_Command>(
                game, std::move(options));
        },
        [&](xd::vec2& position, const std::string& text, const sol::table& table, std::optional<bool> cancelable) {
            std::vector<std::string> choices;
            for (auto& kv : table) {
                choices.push_back(kv.second.as<std::string>());
            }
            Text_Options options{ position };
            set_options(options, choices, text, cancelable);
            auto si = game.get_current_scripting_interface();
            return si->register_choice_command<Show_Text_Command>(
                game, std::move(options));
        }
    );

    // Parsed token types
    lua.new_enum<Token_Type>("Token_Type",
        {
            {"text", Token_Type::TEXT},
            {"opening_tag", Token_Type::OPENING_TAG},
            {"closing_tag", Token_Type::CLOSING_TAG}
        }
    );

    // Parsed text token
    auto token_type = lua.new_usertype<Token>("Token");
    token_type["type"] = sol::readonly(&Token::type);
    token_type["tag"] = sol::readonly(&Token::tag);
    token_type["value"] = sol::readonly(&Token::value);
    token_type["unmatched"] = sol::readonly(&Token::unmatched);
    token_type["start_index"] = sol::readonly(&Token::start_index);
    token_type["end_index"] = sol::readonly(&Token::end_index);
    token_type["self_closing"] = sol::readonly(&Token::self_closing);

    // Text parser
    auto parser_type = lua.new_usertype<Text_Parser>("Text_Parser",
        sol::call_constructor, sol::constructors<Text_Parser()>());
    parser_type["parse"] = [&](Text_Parser& parser, const std::string& text, bool permissive) {
        return sol::as_table(parser.parse(text, permissive));
    };
}
