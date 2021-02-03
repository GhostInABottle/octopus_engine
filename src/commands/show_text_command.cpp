#include "../../include/commands/show_text_command.hpp"
#include "../../include/commands/timed_command.hpp"
#include "../../include/commands/update_canvas_command.hpp"
#include "../../include/xd/audio/sound.hpp"
#include "../../include/game.hpp"
#include "../../include/camera.hpp"
#include "../../include/canvas.hpp"
#include "../../include/map_object.hpp"
#include "../../include/direction.hpp"
#include "../../include/utility/color.hpp"
#include "../../include/utility/string.hpp"
#include "../../include/configurations.hpp"
#include <utility>

struct Show_Text_Command::Impl : Timed_Command {
    explicit Impl(Game& game, Text_Options options) :
            Timed_Command(game, options.duration),
            options(std::move(options)),
            complete(false),
            text_complete(false),
            was_disabled(false),
            cancelable(false),
            selected_choice(0),
            current_choice(0),
            pressed_direction(Direction::NONE),
            press_start(0),
            screen_margins(
                static_cast<float>(Configurations::get<int>("text.screen-edge-margin-x")),
                static_cast<float>(Configurations::get<int>("text.screen-edge-margin-y"))) {

        // Load choice sound effects
        auto audio = game.get_audio();
        auto select_sound_file = Configurations::get<std::string>("audio.choice-select-sfx");
        if (audio && !select_sound_file.empty()) {
            select_sound = std::make_unique<xd::sound>(*audio, select_sound_file);
        }
        auto confirm_sound_file = Configurations::get<std::string>("audio.choice-confirm-sfx");
        if (audio && !confirm_sound_file.empty()) {
            confirm_sound = std::make_unique<xd::sound>(*audio, confirm_sound_file);
        }
        auto cancel_sound_file = Configurations::get<std::string>("audio.choice-cancel-sfx");
        if (audio && !cancel_sound_file.empty()) {
            cancel_sound = std::make_unique<xd::sound>(*audio, cancel_sound_file);
        }

        selected_choice_color = "{color=" + color_to_rgba_string(hex_to_color(
            Configurations::get<std::string>("text.choice-selected-color"))) + "}";

        // Estimate text size by stripping out tags
        auto full = full_text();
        auto clean_text{full};
        int start = 0;
        while ((start = clean_text.find_first_of('{')) != std::string::npos) {
            int end = clean_text.find_first_of('}', start);
            if (end != std::string::npos) {
                clean_text.erase(start, end - start + 1);
            } else {
                break;
            }
        }
        auto text_lines = string_utilities::split(clean_text, "\n", false);
        if (text_lines.empty())
            text_lines.push_back("");

        // Find text width based on widest line
        auto& font_style = game.get_font_style();
        auto text_width = 0.0f;
        for (auto& line : text_lines) {
            auto width = game.get_font()->get_width(line, font_style);
            if (width > text_width)
                text_width = width;
        }

        // Set text position based on size estimation
        float char_height = font_style.line_height();
        float text_height = char_height * (text_lines.size() - 1);
        auto pos{options.position};
        bool camera_relative = (options.position_type & Text_Position_Type::CAMERA_RELATIVE) != Text_Position_Type::NONE;
        bool always_visible = (options.position_type & Text_Position_Type::ALWAYS_VISIBLE) != Text_Position_Type::NONE;

        if (!camera_relative && always_visible) {
            pos -= game.get_camera()->get_position();
        }

        if ((options.position_type & Text_Position_Type::BOTTOM_Y) != Text_Position_Type::NONE) {
            pos.y -= text_height;
        } else {
            pos.y += char_height;
        }

        if (options.centered) {
            pos.x = game.game_width() / 2 - text_width / 2;
        } else if ((options.position_type & Text_Position_Type::CENTERED_X) != Text_Position_Type::NONE) {
            pos.x -= text_width / 2;
        }

        if (always_visible) {
            // Make sure text fits on the screen
            if (pos.x + text_width > game.game_width() - screen_margins.x)
                pos.x = game.game_width() - text_width - screen_margins.x;
            if (pos.x < screen_margins.x)
                pos.x = screen_margins.x;
            if (pos.y + text_height > game.game_height() - screen_margins.y)
                pos.y = game.game_height() - text_height - screen_margins.y;
            if (pos.y < screen_margins.y)
                pos.y = screen_margins.y;
        }

        // Create the text canvas and show it
        canvas = std::make_shared<Canvas>(game, pos, full, camera_relative || always_visible);
        canvas->set_opacity(0.0f);
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

        canvas_updater = std::make_unique<Update_Canvas_Command>(game, *canvas);
        canvas_updater->set_new_opacity(1.0f);
        int duration = options.fade_in_duration == -1 ?
            Configurations::get<int>("text.fade-in-duration") :
            options.fade_in_duration;
        canvas_updater->set_duration(duration);

        if (options.duration == -1) {
            was_disabled = game.get_player()->is_disabled();
            game.get_player()->set_disabled(true);
        }
        cancelable = options.cancelable;

        canvas->set_visible(true);
    }

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

    void update_choice() {
        unsigned int old_choice = current_choice;

        Direction dir = Direction::NONE;
        Direction pressed_dir = Direction::NONE;

        if (game.pressed("down"))
            pressed_dir = Direction::DOWN;
        if (game.pressed("up"))
            pressed_dir = Direction::UP;

        if (pressed_dir != Direction::NONE) {
            if (pressed_dir == pressed_direction) {
                static int delay = Configurations::get<int>("text.choice-press-delay");
                if (game.ticks() - press_start > delay) {
                    pressed_direction = Direction::NONE;
                    press_start = 0;
                    dir = pressed_dir;
                }
            }
            else {
                pressed_direction = pressed_dir;
                press_start = game.ticks();
            }
        }
        else {
            pressed_direction = Direction::NONE;
            press_start = 0;
        }

        if (game.triggered("down"))
            dir = Direction::DOWN;
        if (game.triggered("up"))
            dir = Direction::UP;

        auto choice_count = options.choices.size();
        if (dir == Direction::DOWN)
            current_choice = (current_choice + 1) % choice_count;
        if (dir == Direction::UP)
            current_choice = (current_choice + choice_count - 1) % choice_count;
        if (old_choice != current_choice) {
            if (select_sound) select_sound->play();
            canvas->set_text(full_text());
        }
    }

    std::string cancel_action() {
        static std::string cancel_button = Configurations::get<std::string>("controls.cancel-button");
        if (cancelable && game.triggered_once(cancel_button)) return "cancel";

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

        if (!canvas_updater->is_complete()) {
            canvas_updater->execute();
            return;
        }

        if (text_complete || stopped) {
            if (canvas->is_visible()) {
                canvas->set_visible(false);
            }
            if (options.duration == -1) {
                game.get_player()->set_disabled(was_disabled);
            }
            complete = true;
            return;
        }

        if (options.duration > -1) {
            text_complete = is_done(ticks);
        } else if (!paused) {
            static std::string action_button = Configurations::get<std::string>("controls.action-button");
            
            text_complete = game.triggered_once(action_button);
            if (!options.choices.empty()) {
                if (text_complete) {
                    if (confirm_sound) confirm_sound->play();
                    selected_choice = static_cast<int>(current_choice);
                } else if (auto cancel = cancel_action(); !cancel.empty()) {
                    if (cancel != "pause" && cancel_sound) cancel_sound->play();
                    text_complete = true;
                    selected_choice = -1;
                } else {
                    update_choice();
                }
            }
        }

        if (text_complete) {
            canvas_updater->reset();
            int duration = options.fade_out_duration == -1 ?
                Configurations::get<int>("text.fade-out-duration") :
                options.fade_out_duration;
            canvas_updater->set_duration(duration);
            canvas_updater->set_new_opacity(0.0f);
        }
    }

    void set_start_time(int start) {
        start_time = start;
    }

    Text_Options options;
    std::shared_ptr<Canvas> canvas;
    std::unique_ptr<Update_Canvas_Command> canvas_updater;
    bool complete;
    bool text_complete;
    bool was_disabled;
    bool cancelable;
    int selected_choice;
    unsigned int current_choice;
    Direction pressed_direction;
    long press_start;
    std::string selected_choice_color;
    // Choice navigation sound effect
    std::unique_ptr<xd::sound> select_sound;
    // Choice confirmation sound effect
    std::unique_ptr<xd::sound> confirm_sound;
    // Choice cancel sound effect
    std::unique_ptr<xd::sound> cancel_sound;
    // How far the text should be from the screen edges
    xd::vec2 screen_margins;
};

Show_Text_Command::Show_Text_Command(Game& game, Text_Options options) :
    pimpl(std::make_unique<Impl>(game, std::move(options))) {}

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
