#ifndef HPP_SHOW_TEXT_COMMAND
#define HPP_SHOW_TEXT_COMMAND

#include <vector>
#include <memory>
#include "../xd/graphics/types.hpp"
#include "../command.hpp"

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
    int choice_index();
    void set_start_time(long start);
    static xd::vec2 text_position(Map_Object* object);
private:
    struct Impl;
    friend struct Impl;
    std::unique_ptr<Impl> pimpl;
};

#endif
