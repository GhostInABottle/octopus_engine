#ifndef HPP_SHOW_TEXT_COMMAND
#define HPP_SHOW_TEXT_COMMAND

#include <vector>
#include <memory>
#include "../xd/graphics/types.hpp"
#include "../command.hpp"
#include "../direction.hpp"
#include "update_canvas_command.hpp"

class Game;
class Map_Object;
class Canvas;

enum class Text_Position_Type {
    NONE = 0,
    EXACT_X = 1,
    CENTERED_X = 2,
    EXACT_Y = 4,
    BOTTOM_Y = 8,
    CAMERA_RELATIVE = 16
};

inline Text_Position_Type operator|(Text_Position_Type a, Text_Position_Type b) {
    return static_cast<Text_Position_Type>(static_cast<int>(a) | static_cast<int>(b));
}

inline Text_Position_Type operator&(Text_Position_Type a, Text_Position_Type b) {
    return static_cast<Text_Position_Type>(static_cast<int>(a) & static_cast<int>(b));
}

class Show_Text_Command : public Command {
public:
    Show_Text_Command(Game& game, Map_Object* object,
        const std::string& text, long duration = -1);
    Show_Text_Command(Game& game, Map_Object* object,
        const std::vector<std::string>& choices, const std::string& text = "");
    Show_Text_Command(Game& game, xd::vec2 position,
        const std::vector<std::string>& choices, const std::string& text,
        long duration = -1, bool center = false,
        Text_Position_Type pos_type = Text_Position_Type::EXACT_X
        | Text_Position_Type::EXACT_Y | Text_Position_Type::CAMERA_RELATIVE);
    void execute() override;
    void execute(int ticks) override;
    bool is_complete() const override;
    int choice_index() const { return selected_choice; }
    std::string full_text() const;
    void set_start_time(long start) { this->start_time = start;  }
    static xd::vec2 text_position(Map_Object* object);
private:
    void update_choice();
    Game& game;
    xd::vec2 position;
    std::shared_ptr<Canvas> canvas;
    std::unique_ptr<Update_Canvas_Command> canvas_updater;
    std::string text;
    std::vector<std::string> choices;
    bool complete;
    bool was_disabled;
    unsigned int  selected_choice;
    unsigned int current_choice;
    long start_time;
    long duration;
    Direction pressed_direction;
    long press_start;
};

#endif
