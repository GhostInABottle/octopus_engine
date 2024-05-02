#include "../include/configurations.hpp"
#include "../include/utility/string.hpp"
#include <boost/lexical_cast.hpp>
#include <unordered_set>

void Configurations::load_defaults() {
    defaults["config.version"] = 0;

    defaults["game.title"] = std::string{"Untitled"};
    defaults["game.pause-unfocused"] = true;
    defaults["game.data-folder"] = std::string{};
    defaults["game.data-folder-version"] = std::string{"v0_1"};
    defaults["game.copy-old-data-folder"] = std::string{""};
    defaults["game.object-outline-color"] = std::string{"#FFFFFF00"};
    defaults["game.object-script-preamble"] = std::string{};
    defaults["game.map-loaded-script"] = std::string{};
    defaults["game.pause-script"] = std::string{};
    defaults["game.scripts-folder"] = std::string{};
    defaults["game.store-url"] = std::string{};
    defaults["game.archive-path"] = std::string{};
    defaults["game.icon_base_name"] = std::string{};
    defaults["game.icon_sizes"] = std::string{};

    defaults["text.fade-in-duration"] = 250;
    defaults["text.fade-out-duration"] = 250;
    defaults["text.choice-press-delay"] = 250;
    defaults["text.choice-selected-color"] = std::string{"#FF00FF00"};
    defaults["text.canvas-priority"] = 1000;
    defaults["text.show-background"] = true;
    defaults["text.background-color"] = std::string{"#7F000000"};
    defaults["text.background-margin-left"] = 5;
    defaults["text.background-margin-top"] = 5;
    defaults["text.background-margin-right"] = 5;
    defaults["text.background-margin-bottom"] = 5;
    defaults["text.screen-edge-margin-x"] = 20;
    defaults["text.screen-edge-margin-y"] = 20;

    defaults["graphics.game-width"] = 320;
    defaults["graphics.game-height"] = 240;
    defaults["graphics.screen-width"] = -1;
    defaults["graphics.screen-height"] = -1;
    defaults["graphics.window-width"] = -1;
    defaults["graphics.window-height"] = -1;
    defaults["graphics.logic-fps"] = 60;
    defaults["graphics.canvas-fps"] = 40;
    defaults["graphics.fullscreen"] = false;
    defaults["graphics.vsync"] = false;
    // Scaling modes: aspect, window, stretch, default
    defaults["graphics.scale-mode"] = std::string{"default"};
    defaults["graphics.vertex-shader"] = std::string{};
    defaults["graphics.fragment-shader"] = std::string{};
    defaults["graphics.pause-vertex-shader"] = std::string{};
    defaults["graphics.pause-fragment-shader"] = std::string{};
    defaults["graphics.brightness"] = 1.0f;
    defaults["graphics.contrast"] = 1.0f;
    defaults["graphics.saturation"] = 1.0f;
    defaults["graphics.gamma"] = 1.0f;
    defaults["graphics.use-fbo"] = true;
    defaults["graphics.postprocessing-enabled"] = true;
    defaults["graphics.magnification"] = 1.0f;

    defaults["audio.audio-folder"] = std::string{};
    defaults["audio.music-volume"] = 1.0f;
    defaults["audio.sound-volume"] = 1.0f;
    defaults["audio.choice-select-sfx"] = std::string{};
    defaults["audio.choice-confirm-sfx"] = std::string{};
    defaults["audio.choice-cancel-sfx"] = std::string{};
    defaults["audio.mute-on-pause"] = true;
    defaults["audio.sound-attenuation-factor"] = 50.0f;

    defaults["font.default"] = std::string{};
    defaults["font.bold"] = std::string{};
    defaults["font.italic"] = std::string{};
    defaults["font.size"] = 12;
    defaults["font.line-height"] = 12.0f;
    defaults["font.icon-image"] = std::string{};
    defaults["font.icon-width"] = 12.0f;
    defaults["font.icon-height"] = 12.0f;
    defaults["font.icon-offset-x"] = 0.0f;
    defaults["font.icon-offset-y"] = 0.0f;
    defaults["font.icon-transparent-color"] = std::string{"FF00FF00"};

    defaults["controls.gamepad-enabled"] = true;
    defaults["controls.gamepad-detection"] = true;
    defaults["controls.preferred-gamepad-guid"] = std::string{""};
    defaults["controls.axis-as-dpad"] = true;
    defaults["controls.stick-sensitivity"] = 0.5f;
    defaults["controls.trigger-sensitivity"] = 0.5f;
    defaults["controls.action-button"] = std::string{"a"};
    defaults["controls.cancel-button"] = std::string{"b"};
    defaults["controls.pause-button"] = std::string{"pause"};
    defaults["controls.mapping-file"] = std::string{"keymap.ini"};
    defaults["controls.pause-on-gamepad-disconnect"] = std::string{"auto"};

    defaults["logging.enabled"] = true;
    defaults["logging.filename"] = std::string{"game.log"};
    defaults["logging.level"] = std::string{"debug"};
    defaults["logging.mode"] = std::string{"truncate"};

    defaults["debug.show-fps"] = true;
    defaults["debug.show-time"] = false;
    defaults["debug.pathfinding-sprite"] = std::string{};
    defaults["debug.seed-lua-rng"] = true;
    defaults["debug.save-signature"] = 0x7BEDEADu;
    defaults["debug.update-config-files"] = true;
    // Deprecated configurations, use graphics.[config_name] instead
    defaults["debug.width"] = 320;
    defaults["debug.height"] = 240;
    defaults["debug.magnification"] = 1.0f;
    defaults["debug.logic-fps"] = 60;
    defaults["debug.canvas-fps"] = 40;
    defaults["debug.use-fbo"] = true;

    defaults["player.collision-check-delay"] = 50;
    defaults["player.edge-tolerance-pixels"] = 8;

    defaults["startup.map"] = std::string{};
    defaults["startup.player-sprite"] = std::string{};
    defaults["startup.player-position-x"] = 70.0f;
    defaults["startup.player-position-y"] = 50.0f;
    defaults["startup.tint-color"] = std::string{"00000000"};
    defaults["startup.clear-color"] = std::string{"00000000"};
    defaults["startup.scripts-list"] = std::string{};

    defaults["steam.app-id"] = 0;
    defaults["steam.restart-in-steam"] = false;
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
        if (has_default(key)) {
            if (value_string == "true") value_string = "1";
            if (value_string == "false") value_string = "0";

            auto cast_to_default_type = [&value_string](auto&& default_value) {
                using T = std::decay_t<decltype(default_value)>;
                return Configurations::value_type{ boost::lexical_cast<T>(value_string) };
            };
            values[key] = std::visit(cast_to_default_type, defaults[key]);
            seen_keys.insert(key);
        } else {
            values[key] = value_string;
            seen_keys.insert(key);
        }

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
        return std::visit(visitor, defaults[name]);
    }

    return "";
}
