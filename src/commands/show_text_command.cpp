#include "../../include/xd/audio/sound.hpp"
#include "../../include/commands/show_text_command.hpp"
#include "../../include/commands/update_canvas_command.hpp"
#include "../../include/game.hpp"
#include "../../include/camera.hpp"
#include "../../include/canvas.hpp"
#include "../../include/map_object.hpp"
#include "../../include/direction.hpp"
#include "../../include/utility.hpp"
#include "../../include/configurations.hpp"

struct Show_Text_Command::Impl {
    explicit Impl(Game& game, xd::vec2 position,
        const std::vector<std::string>& choices, const std::string& text,
        long duration, bool center, bool show_dashes, Text_Position_Type pos_type) :
            game(game),
            position(position),
            text(text),
            choices(choices),
            complete(false),
            text_complete(false),
            was_disabled(false),
            selected_choice(0),
            current_choice(0),
            start_time(game.ticks()),
            duration(duration),
            show_dashes(show_dashes),
            pressed_direction(Direction::NONE),
            press_start(0) {

        // Load choice sound effects
        auto audio = game.get_audio();
        auto select_sound_file = Configurations::get<std::string>("game.choice-select-sfx");
        if (audio && !select_sound_file.empty()) {
            select_sound = std::make_unique<xd::sound>(*audio, select_sound_file);
        }
        auto confirm_sound_file = Configurations::get<std::string>("game.choice-confirm-sfx");
        if (audio && !confirm_sound_file.empty()) {
            confirm_sound = std::make_unique<xd::sound>(*audio, confirm_sound_file);
        }

        // Estimate text size by stripping out tags
        auto full = full_text();
        auto clean_text = full;
        int start = 0;
        while ((start = clean_text.find_first_of('{')) != std::string::npos) {
            int end = clean_text.find_first_of('}', start);
            if (end != std::string::npos) {
                clean_text.erase(start, end - start + 1);
            }
        }
        auto text_lines = split(clean_text, "\n");
        if (text_lines.empty())
            text_lines.push_back("");

        // Find longest line to center the text at the right position
        auto longest_line = text_lines[0];
        for (auto& line : text_lines) {
            if (line.size() > longest_line.size())
                longest_line = line;
        }

        // Set text position based on size estimation
        auto& font_style = game.get_font_style();
        float char_height = font_style.line_height();
        float text_width = game.get_font()->get_width(longest_line, font_style);
        float text_height = char_height * text_lines.size();
        auto pos = position;
        bool camera_relative =
            (pos_type & Text_Position_Type::CAMERA_RELATIVE) != Text_Position_Type::NONE;
        if (camera_relative) {
            pos -= game.get_camera()->get_position();
        }

        if ((pos_type & Text_Position_Type::BOTTOM_Y) != Text_Position_Type::NONE) {
            pos.y -= text_height;
        }
        if (center) {
            pos.x = game.game_width() / 2 - text_width / 2;
        }
        else if ((pos_type & Text_Position_Type::CENTERED_X) != Text_Position_Type::NONE) {
            pos.x -= text_width / 2;
        }

        if (camera_relative) {
            // Make sure text fits on the screen
            if (pos.x + text_width > game.game_width() - 10)
                pos.x = static_cast<float>(game.game_width() - text_width - 10);
            if (pos.x < 10.0f)
                pos.x = 10.0f;
            if (pos.y + text_height > game.game_height() - 10)
                pos.y = static_cast<float>(game.game_height() - text_height * 2);
            if (pos.y < 25.0f)
                pos.y = 25.0f;
        }

        // Create the text canvas and show it
        canvas = std::make_shared<Canvas>(game, pos, full, camera_relative);
        canvas->set_opacity(0.0f);
        game.add_canvas(canvas);

        // Show text above other images
        canvas->set_priority(canvas->get_priority() +
            Configurations::get<int>("debug.text-canvas-priority"));
        canvas_updater = std::make_unique<Update_Canvas_Command>(game, *canvas);
        canvas_updater->set_new_opacity(1.0f);
        canvas_updater->set_duration(Configurations::get<int>("game.text-fade-in-duration"));
        if (duration == -1) {
            was_disabled = game.get_player()->is_disabled();
            game.get_player()->set_disabled(true);
        }

        canvas->set_visible(true);
    }

    void play_sound(bool is_confirm) {
        auto& sound = is_confirm ? confirm_sound : select_sound;
        sound->play();
    }

    std::string full_text() const {
        std::string result = text;
        std::string color_prefix = "{color=";
        for (unsigned int i = 0; i < choices.size(); ++i) {
            if (!result.empty())
                result += "\n";
            std::string choice_text = choices[i];
            bool replaced_color = false;
            // Add color for selected choice
            if (i == current_choice) {
                auto start = choices[i].find(color_prefix);
                // Strip existing outermost color, we want green to take precedence
                if (start == 0) {
                    auto end = choices[i].find("}");
                    choice_text.replace(color_prefix.length(),
                        end - color_prefix.length(), "green");
                    replaced_color = true;
                }
                else
                    result += "{color=green}";
            }

            // Add padding before choices if header text was specified
            if (!text.empty())
                result += "  ";

            if (show_dashes)
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
                static int delay = Configurations::get<int>("game.choice-press-delay");
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

        if (dir == Direction::DOWN)
            current_choice = (current_choice + 1) % choices.size();
        if (dir == Direction::UP)
            current_choice = (current_choice + choices.size() - 1) % choices.size();
        if (old_choice != current_choice) {
            play_sound(false);
            canvas->set_text(full_text());
        }
    }

    void execute(int ticks, bool stopped) {
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
            if (duration == -1) {
                game.get_player()->set_disabled(was_disabled);
            }
            complete = true;
            return;
        }

        if (duration > -1) {
            text_complete = ticks > start_time + duration;
        }
        else {
            static std::string action_button =
                Configurations::get<std::string>("controls.action-button");
            text_complete = game.triggered_once(action_button);
            if (!choices.empty()) {
                if (text_complete) {
                    play_sound(true);
                }
                else {
                    update_choice();
                }
            }
        }

        if (text_complete) {
            selected_choice = current_choice;
            canvas_updater->reset();
            canvas_updater->set_duration(Configurations::get<int>("game.text-fade-out-duration"));
            canvas_updater->set_new_opacity(0.0f);
        }
    }

    Game& game;
    xd::vec2 position;
    std::shared_ptr<Canvas> canvas;
    std::unique_ptr<Update_Canvas_Command> canvas_updater;
    std::string text;
    std::vector<std::string> choices;
    bool complete;
    bool text_complete;
    bool was_disabled;
    unsigned int  selected_choice;
    unsigned int current_choice;
    long start_time;
    long duration;
    bool show_dashes;
    Direction pressed_direction;
    long press_start;
    // Choice navigation sound effect
    std::unique_ptr<xd::sound> select_sound;
    // Choice confirmation sound effect
    std::unique_ptr<xd::sound> confirm_sound;
};

Show_Text_Command::Show_Text_Command(Game& game, Text_Options options) :
    pimpl(new Impl(game,
        options.position,
        options.choices,
        options.text,
        options.duration,
        options.centered,
        options.show_dashes,
        options.position_type)) {}

void Show_Text_Command::execute() {
    execute(pimpl->game.ticks());
}

void Show_Text_Command::execute(int ticks) {
    pimpl->execute(ticks, stopped);
}

bool Show_Text_Command::is_complete() const {
    return pimpl->complete;
}
int Show_Text_Command::choice_index() {
    return pimpl->selected_choice;
}
void Show_Text_Command::set_start_time(long start) {
    pimpl->start_time = start;
}
