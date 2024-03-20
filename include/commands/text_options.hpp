#ifndef HPP_TEXT_OPTIONS
#define HPP_TEXT_OPTIONS

#include "../configurations.hpp"
#include "../utility/color.hpp"
#include "../xd/glm.hpp"
#include <string>
#include <vector>

class Map_Object;

enum class Text_Position_Type {
    NONE = 0,
    // Interpret position literally, X refers to left of text
    EXACT_X = 1,
    // The text is centered around the X coordinate
    CENTERED_X = 2,
    // Interpret position literally, Y refers to the top of text
    EXACT_Y = 4,
    // Text sits above the Y coordinate instead of under it
    BOTTOM_Y = 8,
    // The supplied coordinates are relative to the camera instead of the map
    CAMERA_RELATIVE = 16,
    // The text should always be visible, makes map coordinates camera relative
    ALWAYS_VISIBLE = 32
};

inline Text_Position_Type operator|(Text_Position_Type a, Text_Position_Type b) {
    return static_cast<Text_Position_Type>(static_cast<int>(a) | static_cast<int>(b));
}

inline Text_Position_Type operator&(Text_Position_Type a, Text_Position_Type b) {
    return static_cast<Text_Position_Type>(static_cast<int>(a) & static_cast<int>(b));
}

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
    bool cancelable;
    unsigned int choice_indent;
    int canvas_priority;
    int fade_in_duration;
    int fade_out_duration;
    bool background_visible;
    xd::vec4 background_color;
    bool typewriter_on;
    int typewriter_delay;
    std::string typewriter_sound;
    float typewriter_sound_volume;
    float typewriter_sound_pitch;
    float typewriter_sound_max_pitch;
    bool typewriter_skippable;

    Text_Options() :
        object(nullptr),
        position(0.0f, 0.0f),
        position_type(Text_Position_Type::NONE),
        duration(-1),
        centered(false),
        show_dashes(true),
        cancelable(false),
        choice_indent(2),
        canvas_priority(-1),
        fade_in_duration(-1),
        fade_out_duration(-1),
        background_visible(Configurations::get<bool>("text.show-background")),
        background_color(hex_to_color(Configurations::get<std::string>("text.background-color"))),
        typewriter_on(false),
        typewriter_delay(-1),
        typewriter_sound_volume(1.0f),
        typewriter_sound_pitch(1.0f),
        typewriter_sound_max_pitch(-1.0f),
        typewriter_skippable(true) {}


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
            | Text_Position_Type::CAMERA_RELATIVE
            | Text_Position_Type::ALWAYS_VISIBLE);
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

    Text_Options& set_cancelable(bool cancelable) {
        this->cancelable = cancelable;
        return *this;
    }

    Text_Options& set_choice_indent(unsigned int choice_indent) {
        this->choice_indent = choice_indent;
        return *this;
    }

    Text_Options& set_canvas_priority(int priority) {
        this->canvas_priority = priority;
        return *this;
    }

    Text_Options& set_fade_in_duration(int fade_in_duration) {
        this->fade_in_duration = fade_in_duration;
        return *this;
    }

    Text_Options& set_fade_out_duration(int fade_out_duration) {
        this->fade_out_duration = fade_out_duration;
        return *this;
    }

    Text_Options& set_background_visible(bool background_visible) {
        this->background_visible = background_visible;
        return *this;
    }

    Text_Options& set_background_color(xd::vec4 background_color) {
        this->background_color = background_color;
        return *this;
    }

    Text_Options& set_typewriter_on(bool on) {
        this->typewriter_on = on;
        return *this;
    }

    Text_Options& set_typewriter_delay(int delay) {
        this->typewriter_delay = delay;
        return *this;
    }

    Text_Options& set_typewriter_sound(const std::string& sound) {
        this->typewriter_sound = sound;
        return *this;
    }

    Text_Options& set_typewriter_sound_volume(float volume) {
        this->typewriter_sound_volume = volume;
        return *this;
    }

    Text_Options& set_typewriter_sound_pitch(float pitch) {
        this->typewriter_sound_pitch = pitch;
        return *this;
    }

    Text_Options& set_typewriter_sound_max_pitch(float pitch) {
        this->typewriter_sound_max_pitch = pitch;
        return *this;
    }

    Text_Options& set_typewriter_skippable(bool skippable) {
        this->typewriter_skippable = skippable;
        return *this;
    }
};

#endif