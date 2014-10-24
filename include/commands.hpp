#ifndef HPP_COMMANDS
#define HPP_COMMANDS

#include <memory>
#include <vector>
#include <deque>
#include <xd/graphics/types.hpp>
#include "direction.hpp"
#include "command.hpp"
#include "collision_check_types.hpp"

class Map;
class Map_Object;
class Camera;
class Canvas;

class Move_Object_Command : public Command {
public:
    Move_Object_Command(Map_Object& object, Direction dir, float pixels,
        bool skip_blocking, bool change_facing);
    void execute();
    bool is_complete() const;
private:
    Map_Object& object;
    Direction direction;
    float pixels;
    bool skip_blocking;
    bool change_facing;
};

class Dummy_Command : public Simulatable_Command {
    void execute() {}
    bool is_complete() const { return true; }
    bool is_complete(int ticks) const { return true; }
};

struct Node;

class Move_Object_To_Command : public Simulatable_Command {
public:
    Move_Object_To_Command(Map& map, Map_Object& object, float x, float y,
        Collision_Check_Types check_type = Collision_Check_Types::BOTH,
        bool keep_trying = false);
    ~Move_Object_To_Command();
    void execute();
    bool is_complete() const {
        return is_complete(0);
    }
    bool is_complete(int ticks) const;
private:
    void init();
    Map& map;
    Map_Object& object;
    xd::vec2 destination;
    std::deque<Direction> path;
    float pixels;
    bool keep_trying;
    int last_attempt_time;
    bool blocked;
    Collision_Check_Types check_type;
    std::unique_ptr<Node> nearest;
};

struct Sprite_Holder;

class Show_Pose_Command : public Command {
public:
    Show_Pose_Command(Sprite_Holder* holder, const std::string& pose_name, 
        const std::string& state = "", Direction dir = Direction::NONE);
    void execute() {}
    bool is_complete() const;
private:
    Sprite_Holder* holder;
};

class Move_Camera_Command : public Command {
public:
    Move_Camera_Command(Camera& camera, float x, float y, float speed);
    Move_Camera_Command(Camera& camera, Direction dir, float pixels, float speed);
    void execute();
    bool is_complete() const;
private:
    Camera& camera;
    const Map_Object* camera_object;
    xd::vec2 direction;
    float pixels;
    float speed;
};

class Game;

class Tint_Screen_Command : public Command {
public:
    Tint_Screen_Command(Game& game, xd::vec4 color, long duration);
    void execute();
    bool is_complete() const;
private:
    Game& game;
    xd::vec4 old_color;
    xd::vec4 new_color;
    long start_time;
    long duration;
};

class Canvas_Update_Command : public Command {
public:
    Canvas_Update_Command(Game& game, Canvas& canvas, long duration,
        xd::vec2 pos, xd::vec2 mag, float angle, float opacity);
    void execute();
    bool is_complete() const;
private:
    Game& game;
    Canvas& canvas;
    xd::vec2 old_position;
    xd::vec2 old_magnification;
    float old_angle;
    float old_opacity;
    xd::vec2 new_position;
    xd::vec2 new_magnification;
    float new_angle;
    float new_opacity;
    long start_time;
    long duration;
};

struct Layer;

class Layer_Opacity_Update_Command : public Command {
public:
    Layer_Opacity_Update_Command(Game& game, Layer& layer, float opacity, long duration);
    void execute();
    bool is_complete() const;
private:
    Layer& layer;
    Game& game;
    float old_opacity;
    float new_opacity;
    long start_time;
    long duration;
};

namespace xd {
    class music;
}

class Music_Fade_Command  : public Command {
public:
    Music_Fade_Command(Game& game, xd::music& music, float volume, long duration);
    void execute();
    bool is_complete() const;
private:
    xd::music& music;
    Game& game;
    float old_volume;
    float new_volume;
    long start_time;
    long duration;
};

class Shake_Screen_Command  : public Command {
public:
    Shake_Screen_Command(Game& game, float strength, float speed, long duration);
    void execute() {}
    bool is_complete() const;
private:
    Game& game;
    long start_time;
    long duration;
};

class Text_Command : public Command {
public:
    Text_Command(Game& game, Map_Object* object, const std::string& text);
    Text_Command(Game& game, Map_Object* object, std::vector<std::string> choices, const std::string& text = "");
    void execute();
    bool is_complete() const;
    int choice_index() const { return selected_choice; }
    std::string full_text() const;
private:
    void init();
    void update_choice();
    Game& game;
    Map_Object* object;
    std::shared_ptr<Canvas> canvas;
    std::string text;
    std::vector<std::string> choices;
    bool complete;
    bool was_disabled;
    unsigned int  selected_choice;
    unsigned int current_choice;
};

class Wait_Command : public Simulatable_Command {
public:
    Wait_Command(Game& game, long duration, int start = -1);
    void execute() {}
    bool is_complete() const;
    bool is_complete(int ticks) const;
private:
    Game& game;
    long start_time;
    long duration;
};

#endif
