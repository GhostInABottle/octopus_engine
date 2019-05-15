#ifndef HPP_TEXT_OPTIONS
#define HPP_TEXT_OPTIONS

#include <string>
#include <vector>
#include "../xd/graphics/types.hpp"

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

class Map_Object;

struct Text_Options
{
    std::string text;
    std::vector<std::string> choices;
    Map_Object* object;
    xd::vec2 position;
    Text_Position_Type position_type;
    long duration;
    bool centered;
    bool show_dashes;
    bool translated;

    Text_Options() :
        object(nullptr),
        position_type(Text_Position_Type::NONE),
        duration(-1),
        centered(false),
        show_dashes(true),
        translated(false) {}


    Text_Options(Map_Object* object) : Text_Options() {
        set_object(object);
    }

    Text_Options(xd::vec2 position) : Text_Options() {
        set_position(position);
    }

    Text_Options& set_text(const std::string& text) {
        this->text = text;
        return *this;
    }

    Text_Options& set_choices(std::vector<std::string> choices) {
        this->choices = choices;
        return *this;
    }

    Text_Options& set_object(Map_Object* object);

    Text_Options& set_position(xd::vec2 position) {
        this->position = position;
        set_position_type(Text_Position_Type::EXACT_X
            | Text_Position_Type::EXACT_Y
            | Text_Position_Type::CAMERA_RELATIVE);
        return *this;
    }

    Text_Options& set_position_type(Text_Position_Type position_type) {
        this->position_type = position_type;
        return *this;
    }

    Text_Options& set_duration(long duration) {
        this->duration = duration;
        return *this;
    }

    Text_Options& set_centered(bool centered) {
        this->centered = centered;
        return *this;
    }

    Text_Options& set_show_dashes(bool show_dashes) {
        this->show_dashes = show_dashes;
        return *this;
    }
    
    Text_Options& set_translated(bool translated) {
        this->translated = translated;
        return *this;
    }
};

#endif