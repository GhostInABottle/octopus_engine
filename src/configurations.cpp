#include "configurations.hpp"
#include "utility/string.hpp"
#include <boost/lexical_cast.hpp>
#include <string>
#include <type_traits>
#include <unordered_set>

void Configurations::load_defaults() {
    defaults = {
        { "config.version", create_immutable_default(0) },

        { "game.title", create_immutable_default(std::string{"Untitled"}) },
        { "game.pause-unfocused", create_default(true) },
        { "game.data-folder", create_immutable_default(std::string{}) },
        { "game.data-folder-version", create_immutable_default(std::string{"v0_1"}) },
        { "game.copy-old-data-folder", create_immutable_default(std::string{""}) },
        { "game.object-outline-color", create_default(std::string{"#FFFFFF00"}) },
        { "game.object-script-preamble", create_immutable_default(std::string{}) },
        { "game.map-loaded-script", create_immutable_default(std::string{}) },
        { "game.pause-script", create_immutable_default(std::string{}) },
        { "game.scripts-folder", create_immutable_default(std::string{}) },
        { "game.store-url", create_immutable_default(std::string{}) },
        { "game.archive-path", create_immutable_default(std::string{}) },
        { "game.icon_base_name", create_immutable_default(std::string{}) },
        { "game.icon_sizes", create_immutable_default(std::string{}) },

        { "text.fade-in-duration", create_default(250) },
        { "text.fade-out-duration", create_default(250) },
        { "text.choice-press-delay", create_default(250) },
        { "text.choice-selected-color", create_default(std::string{"#FF00FF00"}) },
        { "text.canvas-priority", create_immutable_default(1000) },
        { "text.show-background", create_default(true) },
        { "text.background-color", create_default(std::string{"#7F000000"}) },
        { "text.background-margin-left", create_default(5) },
        { "text.background-margin-top", create_default(5) },
        { "text.background-margin-right", create_default(5) },
        { "text.background-margin-bottom", create_default(5) },
        { "text.screen-edge-margin-x", create_default(20) },
        { "text.screen-edge-margin-y", create_default(20) },

        { "graphics.game-width", create_immutable_default(320) },
        { "graphics.game-height", create_immutable_default(240) },
        { "graphics.screen-width", create_default(-1) },
        { "graphics.screen-height", create_default(-1) },
        { "graphics.window-width", create_default(-1) },
        { "graphics.window-height", create_default(-1) },
        { "graphics.resizable-window", create_default(true) },
        { "graphics.aspect-ratio-numerator", create_default(-1) },
        { "graphics.aspect-ratio-denominator", create_default(-1) },
        { "graphics.maximized-window", create_default(false) },
        { "graphics.logic-fps", create_default(60) },
        { "graphics.canvas-fps", create_default(40) },
        { "graphics.fullscreen", create_default(false) },
        { "graphics.vsync", create_default(false) },
            // Scaling modes: aspect, window, stretch, default
        { "graphics.scale-mode", create_default(std::string{"default"}) },
        { "graphics.vertex-shader", create_default(std::string{}) },
        { "graphics.fragment-shader", create_default(std::string{}) },
        { "graphics.pause-vertex-shader", create_default(std::string{}) },
        { "graphics.pause-fragment-shader", create_default(std::string{}) },
        { "graphics.brightness", create_default(1.0f) },
        { "graphics.contrast", create_default(1.0f) },
        { "graphics.saturation", create_default(1.0f) },
        { "graphics.gamma", create_default(1.0f) },
        { "graphics.use-fbo", create_default(true) },
        { "graphics.postprocessing-enabled", create_default(true) },
        { "graphics.magnification", create_default(1.0f) },

        { "audio.audio-folder", create_immutable_default(std::string{}) },
        { "audio.music-volume", create_default(1.0f) },
        { "audio.sound-volume", create_default(1.0f) },
        { "audio.choice-select-sfx", create_default(std::string{}) },
        { "audio.choice-confirm-sfx", create_default(std::string{}) },
        { "audio.choice-cancel-sfx", create_default(std::string{}) },
        { "audio.mute-on-pause", create_default(true) },
        { "audio.sound-attenuation-factor", create_default(50.0f) },

        { "font.default", create_immutable_default(std::string{}) },
        { "font.bold", create_immutable_default(std::string{}) },
        { "font.italic", create_immutable_default(std::string{}) },
        { "font.size", create_immutable_default(12) },
        { "font.line-height", create_immutable_default(12.0f) },
        { "font.icon-image", create_immutable_default(std::string{}) },
        { "font.icon-width", create_immutable_default(12.0f) },
        { "font.icon-height", create_immutable_default(12.0f) },
        { "font.icon-offset-x", create_immutable_default(0.0f) },
        { "font.icon-offset-y", create_immutable_default(0.0f) },
        { "font.icon-transparent-color", create_immutable_default(std::string{"FF00FF00"}) },

        { "controls.gamepad-enabled", create_default(true) },
        { "controls.gamepad-detection", create_default(true) },
        { "controls.preferred-gamepad-guid", create_default(std::string{""}) },
        { "controls.axis-as-dpad", create_default(true) },
        { "controls.stick-sensitivity", create_default(0.5f) },
        { "controls.trigger-sensitivity", create_default(0.5f) },
        { "controls.action-button", create_default(std::string{"a"}) },
        { "controls.cancel-button", create_default(std::string{"b"}) },
        { "controls.pause-button", create_default(std::string{"pause"}) },
        { "controls.mapping-file", create_immutable_default(std::string{"keymap.ini"}) },
        { "controls.pause-on-gamepad-disconnect", create_default(std::string{"auto"}) },

        { "logging.enabled", create_default(true) },
        { "logging.filename", create_default(std::string{"game.log"}) },
        { "logging.level", create_default(std::string{"debug"}) },
        { "logging.mode", create_default(std::string{"truncate"}) },
        { "logging.file-count", create_default(-1) },
        { "logging.max-file-size-kb", create_default(-1) },

        { "debug.show-fps", create_default(true) },
        { "debug.show-time", create_default(false) },
        { "debug.pathfinding-sprite", create_default(std::string{}) },
        { "debug.seed-lua-rng", create_default(true) },
        { "debug.save-signature", create_immutable_default(0x7BEDEADu) },
        { "debug.update-config-files", create_default(true) },
            // Deprecated configurations, use graphics.[config_name] instead
        { "debug.width", create_default(320) },
        { "debug.height", create_default(240) },
        { "debug.magnification", create_default(1.0f) },
        { "debug.logic-fps", create_default(60) },
        { "debug.canvas-fps", create_default(40) },
        { "debug.use-fbo", create_default(true) },

        { "player.collision-check-delay", create_default(50) },
        { "player.edge-tolerance-pixels", create_default(8) },
        { "player.proximity-distance", create_default(8) },
        { "player.camera-center-offset-x", create_default(0.0f) },
        { "player.camera-center-offset-y", create_default(0.0f) },

        { "startup.map", create_default(std::string{}) },
        { "startup.player-sprite", create_default(std::string{}) },
        { "startup.player-position-x", create_default(70.0f) },
        { "startup.player-position-y", create_default(50.0f) },
        { "startup.tint-color", create_default(std::string{"00000000"}) },
        { "startup.clear-color", create_default(std::string{"00000000"}) },
        { "startup.scripts-list", create_immutable_default(std::string{}) },

        { "steam.app-id", create_immutable_default(0) },
        { "steam.restart-in-steam", create_default(false) }
    };
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
