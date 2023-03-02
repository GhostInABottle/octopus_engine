#include "../../../include/scripting/script_bindings.hpp"
#include "../../../include/scripting/scripting_interface.hpp"
#include "../../../include/game.hpp"
#include "../../../include/map_object.hpp"
#include "../../../include/text_parser.hpp"
#include "../../../include/command_result.hpp"
#include "../../../include/commands/show_text_command.hpp"
#include "../../../include/xd/lua.hpp"
#include "../../../include/xd/glm.hpp"
#include <string>
#include <utility>

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
    options_type["translated"] = sol::readonly(&Text_Options::translated);
    options_type["canvas_priority"] = sol::readonly(&Text_Options::canvas_priority);
    options_type["fade_in_duration"] = sol::readonly(&Text_Options::fade_in_duration);
    options_type["fade_out_duration"] = sol::readonly(&Text_Options::fade_out_duration);
    options_type["background_visible"] = sol::readonly(&Text_Options::background_visible);
    options_type["background_color"] = sol::readonly(&Text_Options::background_color);
    options_type["set_text"] = &Text_Options::set_text;
    options_type["set_object"] = &Text_Options::set_object;
    options_type["set_position"] = &Text_Options::set_position;
    options_type["set_duration"] = &Text_Options::set_duration;
    options_type["set_centered"] = &Text_Options::set_centered;
    options_type["set_show_dashes"] = &Text_Options::set_show_dashes;
    options_type["set_translated"] = &Text_Options::set_translated;
    options_type["set_cancelable"] = &Text_Options::set_cancelable;
    options_type["set_choice_indent"] = &Text_Options::set_choice_indent;
    options_type["set_canvas_priority"] = &Text_Options::set_canvas_priority;
    options_type["set_fade_in_duration"] = &Text_Options::set_fade_in_duration;
    options_type["set_fade_out_duration"] = &Text_Options::set_fade_out_duration;
    options_type["set_background_visible"] = &Text_Options::set_background_visible;
    options_type["set_background_color"] = &Text_Options::set_background_color;
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
