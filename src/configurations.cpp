#include "configurations.hpp"
#include "utility/string.hpp"
#include <boost/lexical_cast.hpp>
#include <string>
#include <type_traits>
#include <unordered_set>

void Configurations::load_defaults() {
    defaults.emplace("config.version", Configurations::Default{ 0, false });

    defaults.emplace("game.title", Configurations::Default{ std::string{"Untitled"}, false });
    defaults.emplace("game.pause-unfocused", Configurations::Default{ true });
    defaults.emplace("game.data-folder", Configurations::Default{ std::string{}, false });
    defaults.emplace("game.data-folder-version", Configurations::Default{ std::string{"v0_1"}, false });
    defaults.emplace("game.copy-old-data-folder", Configurations::Default{ std::string{""}, false });
    defaults.emplace("game.object-outline-color", Configurations::Default{ std::string{"#FFFFFF00"} });
    defaults.emplace("game.object-script-preamble", Configurations::Default{ std::string{}, false });
    defaults.emplace("game.map-loaded-script", Configurations::Default{ std::string{}, false });
    defaults.emplace("game.pause-script", Configurations::Default{ std::string{}, false });
    defaults.emplace("game.scripts-folder", Configurations::Default{ std::string{}, false });
    defaults.emplace("game.store-url", Configurations::Default{ std::string{}, false });
    defaults.emplace("game.archive-path", Configurations::Default{ std::string{}, false });
    defaults.emplace("game.icon_base_name", Configurations::Default{ std::string{}, false });
    defaults.emplace("game.icon_sizes", Configurations::Default{ std::string{}, false });

    defaults.emplace("text.fade-in-duration", Configurations::Default{ 250 });
    defaults.emplace("text.fade-out-duration", Configurations::Default{ 250 });
    defaults.emplace("text.choice-press-delay", Configurations::Default{ 250 });
    defaults.emplace("text.choice-selected-color", Configurations::Default{ std::string{"#FF00FF00"} });
    defaults.emplace("text.canvas-priority", Configurations::Default{ 1000, false });
    defaults.emplace("text.show-background", Configurations::Default{ true });
    defaults.emplace("text.background-color", Configurations::Default{ std::string{"#7F000000"} });
    defaults.emplace("text.background-margin-left", Configurations::Default{ 5 });
    defaults.emplace("text.background-margin-top", Configurations::Default{ 5 });
    defaults.emplace("text.background-margin-right", Configurations::Default{ 5 });
    defaults.emplace("text.background-margin-bottom", Configurations::Default{ 5 });
    defaults.emplace("text.screen-edge-margin-x", Configurations::Default{ 20 });
    defaults.emplace("text.screen-edge-margin-y", Configurations::Default{ 20 });

    defaults.emplace("graphics.game-width", Configurations::Default{ 320, false });
    defaults.emplace("graphics.game-height", Configurations::Default{ 240, false });
    defaults.emplace("graphics.screen-width", Configurations::Default{ -1 });
    defaults.emplace("graphics.screen-height", Configurations::Default{ -1 });
    defaults.emplace("graphics.window-width", Configurations::Default{ -1 });
    defaults.emplace("graphics.window-height", Configurations::Default{ -1 });
    defaults.emplace("graphics.logic-fps", Configurations::Default{ 60 });
    defaults.emplace("graphics.canvas-fps", Configurations::Default{ 40 });
    defaults.emplace("graphics.fullscreen", Configurations::Default{ false });
    defaults.emplace("graphics.vsync", Configurations::Default{ false });
    // Scaling modes: aspect, window, stretch, default
    defaults.emplace("graphics.scale-mode", Configurations::Default{ std::string{"default"} });
    defaults.emplace("graphics.vertex-shader", Configurations::Default{ std::string{} });
    defaults.emplace("graphics.fragment-shader", Configurations::Default{ std::string{} });
    defaults.emplace("graphics.pause-vertex-shader", Configurations::Default{ std::string{} });
    defaults.emplace("graphics.pause-fragment-shader", Configurations::Default{ std::string{} });
    defaults.emplace("graphics.brightness", Configurations::Default{ 1.0f });
    defaults.emplace("graphics.contrast", Configurations::Default{ 1.0f });
    defaults.emplace("graphics.saturation", Configurations::Default{ 1.0f });
    defaults.emplace("graphics.gamma", Configurations::Default{ 1.0f });
    defaults.emplace("graphics.use-fbo", Configurations::Default{ true });
    defaults.emplace("graphics.postprocessing-enabled", Configurations::Default{ true });
    defaults.emplace("graphics.magnification", Configurations::Default{ 1.0f });

    defaults.emplace("audio.audio-folder", Configurations::Default{ std::string{}, false });
    defaults.emplace("audio.music-volume", Configurations::Default{ 1.0f });
    defaults.emplace("audio.sound-volume", Configurations::Default{ 1.0f });
    defaults.emplace("audio.choice-select-sfx", Configurations::Default{ std::string{} });
    defaults.emplace("audio.choice-confirm-sfx", Configurations::Default{ std::string{} });
    defaults.emplace("audio.choice-cancel-sfx", Configurations::Default{ std::string{} });
    defaults.emplace("audio.mute-on-pause", Configurations::Default{ true });
    defaults.emplace("audio.sound-attenuation-factor", Configurations::Default{ 50.0f });

    defaults.emplace("font.default", Configurations::Default{ std::string{}, false });
    defaults.emplace("font.bold", Configurations::Default{ std::string{}, false });
    defaults.emplace("font.italic", Configurations::Default{ std::string{}, false });
    defaults.emplace("font.size", Configurations::Default{ 12, false });
    defaults.emplace("font.line-height", Configurations::Default{ 12.0f, false });
    defaults.emplace("font.icon-image", Configurations::Default{ std::string{}, false });
    defaults.emplace("font.icon-width", Configurations::Default{ 12.0f, false });
    defaults.emplace("font.icon-height", Configurations::Default{ 12.0f, false });
    defaults.emplace("font.icon-offset-x", Configurations::Default{ 0.0f, false });
    defaults.emplace("font.icon-offset-y", Configurations::Default{ 0.0f, false });
    defaults.emplace("font.icon-transparent-color", Configurations::Default{ std::string{"FF00FF00"}, false });

    defaults.emplace("controls.gamepad-enabled", Configurations::Default{ true });
    defaults.emplace("controls.gamepad-detection", Configurations::Default{ true });
    defaults.emplace("controls.preferred-gamepad-guid", Configurations::Default{ std::string{""} });
    defaults.emplace("controls.axis-as-dpad", Configurations::Default{ true });
    defaults.emplace("controls.stick-sensitivity", Configurations::Default{ 0.5f });
    defaults.emplace("controls.trigger-sensitivity", Configurations::Default{ 0.5f });
    defaults.emplace("controls.action-button", Configurations::Default{ std::string{"a"} });
    defaults.emplace("controls.cancel-button", Configurations::Default{ std::string{"b"} });
    defaults.emplace("controls.pause-button", Configurations::Default{ std::string{"pause"} });
    defaults.emplace("controls.mapping-file", Configurations::Default{ std::string{"keymap.ini"}, false });
    defaults.emplace("controls.pause-on-gamepad-disconnect", Configurations::Default{ std::string{"auto"} });

    defaults.emplace("logging.enabled", Configurations::Default{ true });
    defaults.emplace("logging.filename", Configurations::Default{ std::string{"game.log"} });
    defaults.emplace("logging.level", Configurations::Default{ std::string{"debug"} });
    defaults.emplace("logging.mode", Configurations::Default{ std::string{"truncate"} });
    defaults.emplace("logging.file-count", Configurations::Default{ -1 });
    defaults.emplace("logging.max-file-size-kb", Configurations::Default{ -1 });

    defaults.emplace("debug.show-fps", Configurations::Default{ true });
    defaults.emplace("debug.show-time", Configurations::Default{ false });
    defaults.emplace("debug.pathfinding-sprite", Configurations::Default{ std::string{}, false });
    defaults.emplace("debug.seed-lua-rng", Configurations::Default{ true });
    defaults.emplace("debug.save-signature", Configurations::Default{ 0x7BEDEADu, false });
    defaults.emplace("debug.update-config-files", Configurations::Default{ true });
    // Deprecated configurations, use graphics.[config_name] instead
    defaults.emplace("debug.width", Configurations::Default{ 320 });
    defaults.emplace("debug.height", Configurations::Default{ 240 });
    defaults.emplace("debug.magnification", Configurations::Default{ 1.0f });
    defaults.emplace("debug.logic-fps", Configurations::Default{ 60 });
    defaults.emplace("debug.canvas-fps", Configurations::Default{ 40 });
    defaults.emplace("debug.use-fbo", Configurations::Default{ true });

    defaults.emplace("player.collision-check-delay", Configurations::Default{ 50 });
    defaults.emplace("player.edge-tolerance-pixels", Configurations::Default{ 8 });
    defaults.emplace("player.proximity-distance", Configurations::Default{ 8 });
    defaults.emplace("player.camera-center-offset-x", Configurations::Default{ 0.0f });
    defaults.emplace("player.camera-center-offset-y", Configurations::Default{ 0.0f });

    defaults.emplace("startup.map", Configurations::Default{ std::string{} });
    defaults.emplace("startup.player-sprite", Configurations::Default{ std::string{} });
    defaults.emplace("startup.player-position-x", Configurations::Default{ 70.0f });
    defaults.emplace("startup.player-position-y", Configurations::Default{ 50.0f });
    defaults.emplace("startup.tint-color", Configurations::Default{ std::string{"00000000"} });
    defaults.emplace("startup.clear-color", Configurations::Default{ std::string{"00000000"} });
    defaults.emplace("startup.scripts-list", Configurations::Default{ std::string{}, false });

    defaults.emplace("steam.app-id", Configurations::Default{ 0, false });
    defaults.emplace("steam.restart-in-steam", Configurations::Default{ false });
}

std::vector<std::string> Configurations::parse(std::istream& stream, bool is_default) {
    if (defaults.size() == 0) {
        load_defaults();
    }

    if (is_default) {
        section_order.clear();
    }

    add_section("", is_default);

    std::vector<std::string> errors;
    std::string current_section;
    std::string line;
    std::unordered_set<std::string> seen_keys;
    int line_number = -1;

    while (std::getline(stream, line)) {
        line_number++;
        string_utilities::trim(line);

        // Empty line and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            add_to_section_order(section_order.back(), line, is_default);
            continue;
        }

        // Section
        if (line[0] == '[') {
            auto end = line.find(']');
            if (end == std::string::npos) {
                errors.push_back("Config file contains section line without closing ] at line "
                    + std::to_string(line_number) + ", line content: " + line);
            } else {
                current_section = line.substr(1, end - 1);
                string_utilities::trim(current_section);
            }

            add_section(current_section, is_default);
            continue;
        }

        // Key = Value pairs
        auto eq = line.find('=');
        if (eq == std::string::npos) {
            errors.push_back("Config file is missing = sign at line "
                + std::to_string(line_number) + ", line content: " + line);
            continue;
        }

        auto key = line.substr(0, eq);
        string_utilities::trim(key);
        if (key.empty()) {
            errors.push_back("Config file is missing configuration key at line "
                + std::to_string(line_number) + ", line content: " + line);
            continue;
        }

        if (!current_section.empty()) {
            key = current_section + "." + key;
        }

        if (seen_keys.find(key) != std::end(seen_keys)) {
            errors.push_back("Config file contains duplicate key '" + key + "' at line "
                + std::to_string(line_number) + ", line content: " + line);
        }

        // Is the key part of the default config file or config default values
        auto is_default_key = is_default || exists(key);

        auto value_string = line.substr(eq + 1);
        string_utilities::trim(value_string);
        auto has_default_value = has_default(key);
        // Only add keys that are modifiable, except when loading the default config
        auto should_add = is_default || !has_default_value || defaults[key].modifiable;
        if (has_default_value && should_add) {
            if (value_string == "true") value_string = "1";
            if (value_string == "false") value_string = "0";

            auto cast_to_default_type = [&value_string](auto&& default_value) {
                using T = std::decay_t<decltype(default_value)>;
                return Configurations::value_type{ boost::lexical_cast<T>(value_string) };
            };
            values[key] = std::visit(cast_to_default_type, defaults[key].value);
            seen_keys.insert(key);
        } else if (should_add) {
            values[key] = value_string;
            seen_keys.insert(key);
        }

        if (!should_add) continue;

        add_to_section_order(section_order.back(), key, is_default);

        if (!is_default_key) {
            // Add any non-default config keys to the end of the section
            add_to_section_order(key);
        }
    }

    changed_since_save = false;
    if (values.empty()) {
        errors.push_back("Config file was completely empty or invalid");
    }

    return errors;
}

void Configurations::save(std::ostream& stream) {
    for (const auto& [section, lines_or_keys] : section_order) {
        if (lines_or_keys.empty()) continue;

        if (!section.empty()) {
            stream << "[" << section << "]\n";
            if (!stream) {
                throw config_exception("Error writing section " + section + " to config file");
            }
        }

        for (const auto& line_or_key : lines_or_keys) {
            const auto is_line = line_or_key.empty()
                || line_or_key[0] == '#'
                || line_or_key[0] == ';';
            if (is_line) {
                stream << line_or_key << "\n";
                if (!stream) {
                    throw config_exception("Error writing line " + line_or_key + " to config file");
                }
            } else {
                if (!has_value(line_or_key)) continue;
                const auto dot = line_or_key.find(".");
                const auto key = dot == std::string::npos
                    ? line_or_key
                    : line_or_key.substr(dot + 1);
                const auto value = get_string(line_or_key);
                const auto space = value.empty() ? "" : " ";

                stream << key << " =" << space << value << "\n";
                if (!stream) {
                    throw config_exception("Error writing key " + line_or_key + " to config file");
                }
            }
        }
    }

    changed_since_save = false;
}

std::string Configurations::get_string(const std::string& name) {
    auto visitor = [](auto&& arg) { return boost::lexical_cast<std::string>(arg); };
    if (has_value(name)) {
        return std::visit(visitor, values[name]);
    } else if (has_default(name)) {
        return std::visit(visitor, defaults[name].value);
    }

    return "";
}
