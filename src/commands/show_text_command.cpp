#include "show_text_command.hpp"
#include "timed_command.hpp"
#include "update_opacity_command.hpp"
#include "../audio_player.hpp"
#include "../camera.hpp"
#include "../canvas/text_canvas.hpp"
#include "../configurations.hpp"
#include "../direction.hpp"
#include "../game.hpp"
#include "../map/map.hpp"
#include "../map/map_object.hpp"
#include "../text_parser.hpp"
#include "../utility/color.hpp"
#include "../utility/string.hpp"
#include "../xd/audio/sound.hpp"
#include <utility>

struct Show_Text_Command::Impl : Timed_Command {
    // Text display options
    Text_Options options;
    // The canvas for showing the text
    std::shared_ptr<Text_Canvas> canvas;
    // Command to fade the text in/out
    std::unique_ptr<Update_Opacity_Command> canvas_updater;
    // Command is complete when it's stopped or text is done and canvas is hidden
    bool complete;
    // Text is done when pressing enter (or cancel if cancelable) or after duration
    bool text_complete;
    // Was the player disabled before starting this command
    bool was_disabled;
    // Index of the final selected choice, or -1 if canceled
    int selected_choice;
    // Index of the currently active choice
    unsigned int current_choice;
    // The last pressed direction (for detecting long presses)
    Direction pressed_direction;
    // Ticks when the last long press started
    long press_start;
    // A tag such as "{color=green}" based on config
    std::string selected_choice_color;
    // Choice navigation sound effect
    std::shared_ptr<xd::sound> select_sound;
    // Choice confirmation sound effect
    std::shared_ptr<xd::sound> confirm_sound;
    // Choice cancel sound effect
    std::shared_ptr<xd::sound> cancel_sound;
    // How far the text should be from the screen edges
    xd::vec2 screen_margins;
    // Is the typewriter effect done?
    int typewriter_done;

    explicit Impl(Game& game, Text_Options text_options) :
            Timed_Command(game, text_options.duration),
            options(std::move(text_options)),
            complete(false),
            text_complete(false),
            was_disabled(false),
            selected_choice(0),
            current_choice(0),
            pressed_direction(Direction::NONE),
            press_start(0),
            screen_margins(
                static_cast<float>(Configurations::get<int>("text.screen-edge-margin-x")),
                static_cast<float>(Configurations::get<int>("text.screen-edge-margin-y"))),
            typewriter_done(false) {

        // Load choice sound effects
        auto& audio_player = game.get_audio_player();

        if (!options.select_sound.empty()) {
            select_sound = load_sound(audio_player, options.select_sound, "audio.choice-select-sfx");
        }
        if (!options.confirm_sound.empty()) {
            confirm_sound = load_sound(audio_player, options.confirm_sound, "audio.choice-confirm-sfx");
        }
        if (!options.cancel_sound.empty()) {
            cancel_sound = load_sound(audio_player, options.cancel_sound, "audio.choice-cancel-sfx");
        }

        selected_choice_color = "{color=" + color_to_rgba_string(hex_to_color(
            Configurations::get<std::string>("text.choice-selected-color"))) + "}";

        auto full = full_text();
        // Remove typewriter tags because we want to measure the full width of all lines
        std::unordered_set<std::string> tags_to_strip{"typewriter"};
        auto clean_text = Text_Parser::strip_tags(full, tags_to_strip);
        auto text_lines = Text_Parser::split_to_lines(clean_text);

        // Find text width based on widest line
        auto& font_style = game.get_font_style();
        std::vector<float> line_widths;
        auto text_width = 0.0f;
        for (auto& line : text_lines) {
            auto width = game.text_width(line, nullptr, &font_style);
            line_widths.push_back(width);
            if (width > text_width) {
                text_width = width;
            }
        }

        // Set text position based on size estimation
        float char_height = font_style.line_height();
        float text_height = char_height * (text_lines.size() - 1);
        auto pos{options.position};

        auto pos_type = options.position_type;
        bool camera_relative = (pos_type & Text_Position_Type::CAMERA_RELATIVE) != Text_Position_Type::NONE;
        bool always_visible = (pos_type & Text_Position_Type::ALWAYS_VISIBLE) != Text_Position_Type::NONE;

        if (!camera_relative && always_visible) {
            pos -= game.get_camera()->get_pixel_position();
        }

        if ((pos_type & Text_Position_Type::BOTTOM_Y) != Text_Position_Type::NONE) {
            pos.y -= text_height;
        } else {
            pos.y += char_height;
        }

        auto game_width = static_cast<float>(game.game_width());
        auto x_centered = (pos_type & Text_Position_Type::CENTERED_X) != Text_Position_Type::NONE;
        if (x_centered) {
            // The text box as a whole is centered around the x coordinate
            pos.x -= text_width / 2;
        } else if (options.centered) {
            // Each line is centered around the x coordinate or the center of the screen
            pos.x = game_width / 2 - text_width / 2;
        }

        if (always_visible) {
            // Make sure text fits on the screen
            if (pos.x + text_width > game_width - screen_margins.x) {
                pos.x = game_width - text_width - screen_margins.x;
            }
            if (pos.x < screen_margins.x) {
                pos.x = screen_margins.x;
            }

            auto game_height = static_cast<float>(game.game_height());
            if (pos.y + text_height > game_height - screen_margins.y) {
                pos.y = game_height - text_height - screen_margins.y;
            }
            if (pos.y < screen_margins.y) {
                pos.y = screen_margins.y;
            }
        }

        // Set typewriter options if needed
        std::optional<Text_Canvas::Typewriter_Options> typewriter_options;
        if (options.typewriter_on) {
            typewriter_options = Text_Canvas::Typewriter_Options{};
            typewriter_options->slot = game.get_map()->next_typewriter_slot();
            typewriter_options->delay = options.typewriter_delay;
            typewriter_options->sound_filename = options.typewriter_sound;
            typewriter_options->sound_volume = options.typewriter_sound_volume;
            typewriter_options->sound_pitch = options.typewriter_sound_pitch;
            typewriter_options->sound_max_pitch = options.typewriter_sound_max_pitch;
        }

        // Create the text canvas and show it
        canvas = std::make_shared<Text_Canvas>(game, pos, full,
            camera_relative || always_visible, typewriter_options);
        canvas->set_opacity(0.0f);

        if (options.centered) {
            canvas->set_x(pos.x + text_width / 2);
            canvas->set_centered(true, std::move(line_widths));
        }

        if (options.background_visible) {
            canvas->set_background_visible(true);
            canvas->set_background_color(options.background_color);
            canvas->set_background_rect(xd::rect{pos.x, pos.y - char_height,
                text_width, text_height + char_height});
        }

        game.add_canvas(canvas);

        // Show text above other images
        int priority = canvas->get_priority();
        priority += options.canvas_priority == -1 ?
            Configurations::get<int>("text.canvas-priority") :
            options.canvas_priority;
        canvas->set_priority(priority);

        int duration = options.fade_in_duration == -1 ?
            Configurations::get<int>("text.fade-in-duration") :
            options.fade_in_duration;
        canvas_updater = std::make_unique<Update_Opacity_Command>(game, *canvas, 1.0f, duration);

        if (options.duration == -1) {
            was_disabled = game.get_player()->is_disabled();
            game.get_player()->set_disabled(true);
        }

        canvas->set_visible(true);
    }

    std::shared_ptr<xd::sound> load_sound(Audio_Player& audio_player,
            const std::string& filename, const std::string& config) {
        if (filename == "default") {
            return audio_player.load_global_config_sound(config, 3, false);
        } else {
            return audio_player.load_global_sound(filename, 3, false);
        }
    }

    // The text along with colored/indented choices
    std::string full_text() const {
        std::string result = options.text;
        for (unsigned int i = 0; i < options.choices.size(); ++i) {
            if (!result.empty())
                result += "\n";
            std::string choice_text = options.choices[i];
            bool replaced_color = false;
            // Add color for selected choice
            if (i == current_choice) {
                auto start = choice_text.find("{color=");
                // Strip existing outermost color, selected color takes precedence
                if (start == 0 && string_utilities::ends_with(choice_text, "{/color}")) {
                    auto end = choice_text.find("}");
                    choice_text.replace(0, end + 1, "");
                    replaced_color = true;
                }
                result += selected_choice_color;
            }

            // Add padding before choices if header text was specified
            if (!options.text.empty() && options.choice_indent > 0)
                result += std::string(options.choice_indent, ' ');

            if (options.show_dashes)
                result += "- ";

            result += choice_text;

            if (i == current_choice && !replaced_color)
                result += "{/color}";
        }
        return result;
    }

    // Update the active choice based on input
    void update_choice(bool check_pressed) {
        unsigned int old_choice = current_choice;

        Direction dir = Direction::NONE;
        Direction pressed_dir = Direction::NONE;

        if (game.pressed("down")) {
            pressed_dir = Direction::DOWN;
        } else if (game.pressed("up")) {
            pressed_dir = Direction::UP;
        }

        if (check_pressed && pressed_dir != Direction::NONE) {
            // Track long presses
            auto ticks = game.is_paused() ? game.window_ticks() : game.ticks();
            if (pressed_dir == pressed_direction) {
                static int delay = Configurations::get<int>("text.choice-press-delay");
                if (ticks - press_start > delay) {
                    pressed_direction = Direction::NONE;
                    press_start = 0;
                    dir = pressed_dir;
                }
            }
            else {
                pressed_direction = pressed_dir;
                press_start = ticks;
            }
        }
        else {
            pressed_direction = Direction::NONE;
            press_start = 0;
        }

        if (game.triggered("down")) {
            dir = Direction::DOWN;
        } else if (game.triggered("up")) {
            dir = Direction::UP;
        }

        auto choice_count = options.choices.size();
        if ((dir & Direction::DOWN) != Direction::NONE) {
            current_choice = (current_choice + 1) % choice_count;
        } else if ((dir & Direction::UP) != Direction::NONE) {
            current_choice = (current_choice + choice_count - 1) % choice_count;
        }

        // Update the choice colors
        if (old_choice != current_choice) {
            if (select_sound) select_sound->play();
            canvas->set_text(full_text());
        }
    }

    // Check if the cancel or pause button were pressed
    std::string cancel_action() {
        static std::string cancel_button = Configurations::get<std::string>("controls.cancel-button");
        if (options.cancelable && game.triggered_once(cancel_button)) return "cancel";

        static std::string pause_button = Configurations::get<std::string>("controls.pause-button");
        if (game.is_paused() && game.triggered_once(pause_button)) return "pause";

        return "";
    }

    void execute() override {}
    bool is_complete() const override { return complete; };

    void process(bool stopped) {
        return process(game.ticks(), stopped);
    }

    void process(int ticks, bool stopped) {
        if (complete) {
            return;
        }

        // Wait for text to be shown or hidden
        auto has_duration = options.duration > -1;
        auto updating_canvas = !canvas_updater->is_complete();
        if (updating_canvas) {
            if (stopped) canvas_updater->stop();
            canvas_updater->execute();

            auto mostly_visible = !text_complete && !has_duration
                && canvas->get_opacity() >= 0.7f;
            if (!force_stopped && !mostly_visible) {
                // Allow updating selected choice while the canvas is being shown
                if (!paused && !text_complete && !options.choices.empty()) {
                    update_choice(false);
                }
                return;
            }
        }

        // Mark the command as complete when text is done / stopped
        if (text_complete || stopped) {
            if (stopped) canvas_updater->stop();
            if (canvas->is_visible()) {
                canvas->set_visible(false);
            }

            if (!has_duration) {
                game.get_player()->set_disabled(was_disabled);
            }

            complete = true;
            return;
        }

        static std::string action_button = Configurations::get<std::string>("controls.action-button");

        // Wait for the typewriter effect to finish
        if (options.typewriter_on && !canvas->typewriter_done()) {
            if (options.typewriter_skippable && !paused && game.triggered_once(action_button)) {
                canvas->skip_typewriter();
                typewriter_done = true;
            }
            return;
        } else if (options.typewriter_on && has_duration && !typewriter_done) {
            // If there was a duration, start waiting for it after the typewriter effect is done
            typewriter_done = true;
            start_time = ticks;
        }

        if (has_duration) {
            text_complete = is_done(ticks);
        } else if (!paused) {
            // Check if the action button is pressed
            text_complete = game.triggered_once(action_button);

            // Handle choices
            if (!options.choices.empty()) {
                if (text_complete) {
                    if (confirm_sound) confirm_sound->play();
                    selected_choice = static_cast<int>(current_choice);
                } else if (auto cancel = cancel_action(); !cancel.empty()) {
                    if (cancel != "pause" && cancel_sound) cancel_sound->play();
                    text_complete = true;
                    selected_choice = -1;
                } else {
                    update_choice(true);
                }
            }
        }

        // Start fading out
        if (text_complete) {
            int duration = options.fade_out_duration == -1 ?
                Configurations::get<int>("text.fade-out-duration") :
                options.fade_out_duration;
            canvas_updater->restart(0.0f, duration);
        }
    }

    void set_start_time(int start) {
        start_time = start;
    }
};

Show_Text_Command::Show_Text_Command(Game& game, Text_Options options) :
        pimpl(std::make_unique<Impl>(game, std::move(options))) {
    map_ptr = game.get_map();
}

Show_Text_Command::~Show_Text_Command() {}

void Show_Text_Command::execute() {
    pimpl->process(stopped);
}

void Show_Text_Command::execute(int ticks) {
    pimpl->process(ticks, stopped);
}

bool Show_Text_Command::is_complete() const {
    return pimpl->is_complete();
}

int Show_Text_Command::choice_index() {
    return pimpl->selected_choice;
}

void Show_Text_Command::set_start_time(int start) {
    pimpl->set_start_time(start);
}

void Show_Text_Command::pause() noexcept {
    pimpl->pause();
}

void Show_Text_Command::pause(int ticks) noexcept {
    pimpl->pause(ticks);
}

void Show_Text_Command::resume() noexcept {
    pimpl->resume();
}
