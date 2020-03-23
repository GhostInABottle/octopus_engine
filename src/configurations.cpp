#include "../include/configurations.hpp"
#include "../include/log.hpp"
#include "../include/utility/file.hpp"
#include "../include/utility/string.hpp"
#include <boost/lexical_cast.hpp>
#include <typeinfo>
#include <fstream>

std::unordered_map<std::string, Configurations::value_type> Configurations::values;
std::unordered_map<std::string, Configurations::value_type> Configurations::defaults;
std::unordered_map<std::string, Configurations::callback> Configurations::observers;

void Configurations::load_defaults() {
    defaults["game.title"] = std::string("Untitled");
    defaults["game.pause-unfocused"] = true;
    defaults["game.data-folder"] = std::string();
    defaults["game.text-fade-in-duration"] = 250;
    defaults["game.text-fade-out-duration"] = 250;
    defaults["game.choice-press-delay"] = 250;
    defaults["game.object-outline-color"] = std::string("#FFFFFF00");
    defaults["game.map-loaded-script"] = std::string();

    defaults["graphics.screen-width"] = 640;
    defaults["graphics.screen-height"] = 480;
    defaults["graphics.fullscreen"] = false;
    // Scaling modes: aspect, window, stretch
    defaults["graphics.scale-mode"] = std::string("window");
    defaults["graphics.vertex-shader"] = std::string();
    defaults["graphics.fragment-shader"] = std::string();
    defaults["graphics.pause-vertex-shader"] = std::string();
    defaults["graphics.pause-fragment-shader"] = std::string();
    defaults["graphics.brightness"] = 1.0f;
    defaults["graphics.contrast"] = 1.0f;
    defaults["graphics.postprocessing-enabled"] = true;

    defaults["audio.music-volume"] = 1.0f;
    defaults["audio.sound-volume"] = 1.0f;
    defaults["audio.choice-select-sfx"] = std::string();
    defaults["audio.choice-confirm-sfx"] = std::string();

    defaults["font.default"] = std::string();
    defaults["font.bold"] = std::string();
    defaults["font.italic"] = std::string();
    defaults["font.size"] = 12;
    defaults["font.line-height"] = 12.0f;

    defaults["controls.gamepad-enabled"] = true;
    defaults["controls.gamepad-detection"] = true;
    defaults["controls.gamepad-number"] = -1;
    defaults["controls.axis-as-dpad"] = true;
    defaults["controls.axis-sensitivity"] = 0.5f;
    defaults["controls.action-button"] = std::string("a");
    defaults["controls.mapping-file"] = std::string("keymap.ini");

    defaults["logging.enabled"] = true;
    defaults["logging.filename"] = std::string("game.log");
    defaults["logging.level"] = std::string("debug");
    defaults["logging.mode"] = std::string("truncate");

    defaults["debug.width"] = 320.0f;
    defaults["debug.height"] = 240.0f;
    defaults["debug.magnification"] = 1.0f;
    defaults["debug.show-fps"] = true;
    defaults["debug.show-time"] = false;
    defaults["debug.logic-fps"] = 40;
    defaults["debug.time-multiplier"] = 0.5f;
    defaults["debug.pathfinding-sprite"] = std::string();
    defaults["debug.canvas-fps"] = 40;
    defaults["debug.use-fbo"] = true;
    defaults["debug.seed-lua-rng"] = true;
    defaults["debug.save-signature"] = 0x7BEDEADu;
    defaults["debug.text-canvas-priority"] = 1000;

    defaults["startup.map"] = std::string();
    defaults["startup.player-sprite"] = std::string();
    defaults["startup.player-position-x"] = 70.0f;
    defaults["startup.player-position-y"] = 50.0f;
    defaults["startup.tint-color"] = std::string("00000000");
    defaults["startup.clear-color"] = std::string("00000000");
    defaults["startup.scripts-list"] = std::string();
}

std::vector<std::string> Configurations::parse(std::string filename) {
    if (defaults.size() == 0) {
        load_defaults();
    }
    normalize_slashes(filename);
    std::ifstream stream(filename);
    if (!stream) {
        throw config_exception("Couldn't read file " + filename);
    }
    values.clear();
    std::vector<std::string> errors;
    std::string current_section;
    std::string line;
    int line_number = -1;
    while (std::getline(stream, line)) {
        line_number++;
        trim(line);
        // Empty line and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        // Section
        if (line[0] == '[') {
            auto end = line.find(']');
            if (end == std::string::npos) {
                errors.push_back(filename + " contains section line without closing ] at line "
                    + std::to_string(line_number) + ", line content: " + line);
            } else {
                current_section = line.substr(1, end - 1);
                trim(current_section);
            }

            continue;
        }

        // Key = Value pairs
        auto eq = line.find('=');
        if (eq == std::string::npos) {
            errors.push_back(filename + " is missing = sign at line "
                + std::to_string(line_number) + ", line content: " + line);
        } else {
            auto key = line.substr(0, eq);
            trim(key);
            if (key.empty()) {
                errors.push_back(filename + " is missing configuration key at line "
                    + std::to_string(line_number) + ", line content: " + line);
                continue;
            }
            if (!current_section.empty()) {
                key = current_section + "." + key;
            }
            if (values.find(key) != values.end()) {
                errors.push_back(filename + " contains duplicate key '" + key + "' at line "
                    + std::to_string(line_number) + ", line content: " + line);
            }
            auto value_string = line.substr(eq + 1);
            trim(value_string);
            if (defaults.find(key) != defaults.end()) {
                if (value_string == "true") value_string = "1";
                if (value_string == "false") value_string = "0";
                values[key] = std::visit(
                    [&value_string](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        return Configurations::value_type{ boost::lexical_cast<T>(value_string) };
                    },
                    defaults[key]);
            } else {
                values[key] = value_string;
            }
        }
    }

    if (values.empty()) {
        errors.push_back(filename + " config file was completely empty or invalid");
    }

    return errors;
}

void Configurations::save(std::string filename) {
    normalize_slashes(filename);
    std::ofstream stream(filename);
    if (!stream) {
        throw config_exception("Couldn't open config file for saving " + filename);
    }
    LOGGER_I << "Saving config file " << filename;
    std::unordered_map<std::string, std::vector<std::pair<std::string, Configurations::value_type>>> sections;
    for (auto& [full_key, value] : values) {
        auto dot = full_key.find(".");
        if (dot == std::string::npos) {
            sections["global"].emplace_back(full_key, value);
        } else {
            auto section = full_key.substr(0, dot);
            auto key = full_key.substr(dot + 1);
            sections[section].emplace_back(key, value);
        }
    }
    for (auto& [section, section_values] : sections) {
        if (section != "global") {
            stream << "[" << section << "]\n";
        }
        if (!stream) {
            throw config_exception("Error writing section " + section + " to config file " + filename);
        }
        for (auto& [key, val] : section_values) {
            auto full_key = section == "global" ? key : section + "." + key;
            if (values.find(full_key) != values.end()) {
                stream << key << " = " << get_string(full_key) << "\n";
                if (!stream) {
                    throw config_exception("Error writing key " + full_key + " to config file " + filename);
                }
            }
        }
    }
}

std::string Configurations::get_string(const std::string& name) {
    auto visitor = [](auto&& arg) { return boost::lexical_cast<std::string>(arg); };
    if (values.find(name) != values.end()) {
        return std::visit(visitor, values[name]);
    } else if (defaults.find(name) != defaults.end()) {
        return std::visit(visitor, defaults[name]);
    }
    return "";
}
