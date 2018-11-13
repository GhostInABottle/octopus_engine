#include "../../include/commands/show_text_command.hpp"
#include "../../include/game.hpp"
#include "../../include/camera.hpp"
#include "../../include/canvas.hpp"
#include "../../include/map_object.hpp"
#include "../../include/utility.hpp"
#include "../../include/configurations.hpp"

Show_Text_Command::Show_Text_Command(Game& game, Map_Object* object,
    const std::string& text, long duration) :
    Show_Text_Command(game, text_position(object), std::vector<std::string>{},
        text, duration, false,
        Text_Position_Type::CENTERED_X
        | Text_Position_Type::BOTTOM_Y
        | Text_Position_Type::CAMERA_RELATIVE) {}

Show_Text_Command::Show_Text_Command(Game& game, Map_Object* object,
    const std::vector<std::string>& choices, const std::string& text) :
    Show_Text_Command(game, text_position(object), choices, text, -1, false,
        Text_Position_Type::CENTERED_X
        | Text_Position_Type::BOTTOM_Y
        | Text_Position_Type::CAMERA_RELATIVE) {}

Show_Text_Command::Show_Text_Command(Game& game, xd::vec2 position,
    const std::vector<std::string>& choices, const std::string& text,
    long duration, bool center, Text_Position_Type pos_type) :
        game(game), position(position), choices(choices), text(text),
        duration(duration), start_time(game.ticks()), complete(false) {
    selected_choice = 0;
    current_choice = 0;
    auto full = full_text();
    // Estimate text size by stripping out tags
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
        pos.x = game.game_width / 2 - text_width / 2;
    } else if ((pos_type & Text_Position_Type::CENTERED_X) != Text_Position_Type::NONE) {
        pos.x -= text_width / 2;
    }

    if (camera_relative) {
        // Make sure text fits on the screen
        if (pos.x + text_width > Game::game_width - 10)
            pos.x = static_cast<float>(Game::game_width - text_width - 10);
        if (pos.x < 10.0f)
            pos.x = 10.0f;
        if (pos.y + text_height > Game::game_height - 10)
            pos.y = static_cast<float>(Game::game_height - text_height * 2);
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
    canvas_updater->set_duration(Configurations::get<int>("game.text-fade-duration"));
    if (duration == -1) {
        was_disabled = game.get_player()->is_disabled();
        game.get_player()->set_disabled(true);
    }

    canvas->set_visible(true);
}

void Show_Text_Command::execute() {
    execute(game.ticks());
}

void Show_Text_Command::execute(int ticks) {
    if (is_complete()) {
        if (canvas->is_visible()) {
            canvas->set_visible(false);
            if (duration == -1) {
                game.get_player()->set_disabled(was_disabled);
            }
        }
        return;
    }

    if (!canvas_updater->is_complete()) {
        canvas_updater->execute();
        return;
    }

    if (duration > -1) {
        complete = ticks > start_time + duration;
    } else {
        static std::string action_button =
            Configurations::get<std::string>("controls.action-button");
        complete = game.triggered_once(action_button);
        if (complete) {
            selected_choice = current_choice;
        } else if (!choices.empty()) {
            update_choice();
        }
    }

    if (complete) {
        canvas_updater->reset();
        canvas_updater->set_new_opacity(0.0f);
    }
}

void Show_Text_Command::update_choice() {
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
        } else {
            pressed_direction = pressed_dir;
            press_start = game.ticks();
        }
    } else {
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
    if (old_choice != current_choice)
        canvas->set_text(full_text());
}

xd::vec2 Show_Text_Command::text_position(Map_Object* object) {
    return object->get_position() + xd::vec2(16, 0);
}

bool Show_Text_Command::is_complete() const {
    return canvas_updater->is_complete() && (stopped || complete);
}

std::string Show_Text_Command::full_text() const {
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
        result += "- " + choice_text;
        if (i == current_choice && !replaced_color)
            result += "{/color}";
    }
    return result;
}
