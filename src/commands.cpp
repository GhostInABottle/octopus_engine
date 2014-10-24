#include "../include/commands.hpp"
#include "../include/map_object.hpp"
#include "../include/layer.hpp"
#include "../include/game.hpp"
#include "../include/map.hpp"
#include "../include/canvas.hpp"
#include "../include/camera.hpp"
#include "../include/utility.hpp"
#include "../include/sprite_data.hpp"
#include "../include/pathfinder.hpp"
#include "../include/direction_utilities.hpp"
#include "../include/configurations.hpp"
#include <xd/audio.hpp>
#include <algorithm>

namespace {
    inline float calculate_alpha(long current, long start, long duration) {
        float alpha = 1.0f;
        if (duration != 0) {
            alpha = static_cast<float>(current - start) / duration;
            alpha = std::min(std::max(alpha, 0.0f), 1.0f);
        }
        return alpha;
    }
}

Command::~Command() {
}

Move_Object_Command::Move_Object_Command(Map_Object& object, Direction dir,
            float pixels, bool skip_blocking, bool change_facing)
            : object(object), direction(dir), pixels(pixels),
            skip_blocking(skip_blocking), change_facing(change_facing) {
    if (direction == Direction::FORWARD)
        direction = object.get_direction();
    else if (direction == Direction::BACKWARD) {
        direction = opposite_direction(object.get_direction());
        this->change_facing = false;
    }
}

void Move_Object_Command::execute() {
    auto collision = object.move(direction, object.get_speed(),
        Collision_Check_Types::BOTH, change_facing);
    if (collision.passable())
        pixels -= object.get_speed();
    else if (skip_blocking)
        pixels = 0.0f;
    if (is_complete())
        object.update_state("FACE");
}

bool Move_Object_Command::is_complete() const {
    return object.is_stopped() || pixels <= 0;
}

Move_Object_To_Command::Move_Object_To_Command(Map& map, Map_Object& object,
    float x, float y, Collision_Check_Types check_type, bool keep_trying)
        : map(map), object(object), destination(x, y), pixels(0.0f),
        check_type(check_type), keep_trying(keep_trying), last_attempt_time(0),
        blocked(false), nearest(nullptr) {}

Move_Object_To_Command::~Move_Object_To_Command() {}

void Move_Object_To_Command::execute() {
    if ((blocked || path.empty()) && keep_trying) {
        object.update_state("FACE");
        int time_passed = map.get_game().ticks() - last_attempt_time;
        if ((map.get_objects_moved() && time_passed > 1000) || time_passed > 5000) {
            init();
            map.set_objects_moved(false);
            blocked = false;
        }
        return;
    }
    if (path.empty()) {
        return;
    }
    int index = static_cast<int>(pixels) / map.get_tile_width();
    int max_index = static_cast<int>(path.size() - 1);
    if (index > max_index && !is_complete()) {
        blocked = true;
        return;
    }
    // TODO: Temp fix for back alley collision bug
    if (check_type == Collision_Check_Types::TILE)
        object.set_passthrough(true);
    auto collision = object.move(path[index], object.get_speed(), check_type);
    if (check_type == Collision_Check_Types::TILE)
        object.set_passthrough(false);
    if (collision.passable())
        pixels += object.get_speed();
    else
        blocked = true;
}

void Move_Object_To_Command::init() {
    Pathfinder finder(map, object, destination, 0, true, check_type);
    if (nearest)
        finder.nearest() = *nearest;
    finder.calculate_path();
    if (finder.nearest().h > 0 && 
        (!nearest || finder.nearest().h < nearest->h)) {
        nearest.reset(new Node(finder.nearest()));
        nearest->parent = nullptr;
    }
    path = finder.generate_path();
    pixels = 0.0f;
    last_attempt_time = map.get_game().ticks();
}

bool Move_Object_To_Command::is_complete(int ticks) const {
    auto pos = object.get_real_position();
    bool complete = object.is_stopped() || (!keep_trying && path.empty()) ||
        (std::abs(pos.x - destination.x) < 8.0f &&
        std::abs(pos.y - destination.y) < 8.0f);
    if (complete)
        object.update_state("FACE");
    return complete;
}

Show_Pose_Command::Show_Pose_Command(Sprite_Holder* holder,
        const std::string& pose_name, const std::string& state,
        Direction dir) : holder(holder) {
    holder->set_pose(pose_name, state, dir);
}

bool Show_Pose_Command::is_complete() const {
    auto sprite = holder->get_sprite();
    if (sprite->get_pose().repeats == -1)
        return true;
    return sprite->is_stopped();
}

Move_Camera_Command::Move_Camera_Command(Camera& camera, float x, float y, float speed) 
        : camera(camera), camera_object(camera.get_object()), speed(speed) {
    camera.set_object(nullptr);
    xd::vec2 displacement = xd::vec2(x, y) - camera.get_position();
    direction = xd::normalize(displacement);
    pixels = static_cast<float>(xd::length(displacement));
}

Move_Camera_Command::Move_Camera_Command(Camera& camera, Direction dir, float pixels, float speed)
        : camera(camera), camera_object(camera.get_object()), pixels(pixels), speed(speed) {
    camera.set_object(nullptr);
    direction = direction_to_vector(dir);
}

void Move_Camera_Command::execute() {
    xd::vec2 new_position = camera.get_position() + direction * speed;
    camera.set_position(new_position.x, new_position.y);
    pixels -= speed;
}

bool Move_Camera_Command::is_complete() const {
    return check_close(pixels, 0.0f, 1.0f);
}

Tint_Screen_Command::Tint_Screen_Command(Game& game, xd::vec4 color, long duration) :
    game(game), old_color(game.get_camera()->get_tint_color()), new_color(color),
    start_time(game.ticks()), duration(duration) {}

void Tint_Screen_Command::execute() {
    float alpha = calculate_alpha(game.ticks(), start_time, duration);
    game.get_camera()->set_tint_color(lerp(old_color, new_color, alpha));
}

bool Tint_Screen_Command::is_complete() const {
    return game.ticks() - start_time > duration;
}

Canvas_Update_Command::Canvas_Update_Command(Game& game, Canvas& canvas,
    long duration, xd::vec2 pos, xd::vec2 mag, float angle, float opacity) :
    game(game),
    canvas(canvas), 
    old_position(canvas.get_position()),
    old_magnification(canvas.get_magnification()), 
    old_angle(static_cast<float>(canvas.get_angle())),
    old_opacity(canvas.get_opacity()),
    new_position(pos),
    new_magnification(mag),
    new_angle(angle),
    new_opacity(opacity),
    start_time(game.ticks()),
    duration(duration) {}

void Canvas_Update_Command::execute() {
    float alpha = calculate_alpha(game.ticks(), start_time, duration);
    canvas.set_position(lerp(old_position, new_position, alpha));
    canvas.set_magnification(lerp(old_magnification, new_magnification, alpha));
    canvas.set_angle(lerp(old_angle, new_angle, alpha));
    canvas.set_opacity(lerp(old_opacity, new_opacity, alpha));
}

bool Canvas_Update_Command::is_complete() const {
    return game.ticks() - start_time > duration;
}


Layer_Opacity_Update_Command::Layer_Opacity_Update_Command(Game& game, Layer& layer, float opacity, long duration) :
    game(game),
    layer(layer),
    old_opacity(layer.opacity),
    new_opacity(opacity),
    start_time(game.ticks()),
    duration(duration) {}

void Layer_Opacity_Update_Command::execute() {
    float alpha = calculate_alpha(game.ticks(), start_time, duration);
    layer.opacity= lerp(old_opacity, new_opacity, alpha);
}

bool Layer_Opacity_Update_Command::is_complete() const {
    return game.ticks() - start_time > duration;
}

Music_Fade_Command::Music_Fade_Command(Game& game, xd::music& music, float volume, long duration) :
    game(game),
    music(music),
    old_volume(music.get_volume()),
    new_volume(volume),
    start_time(game.ticks()),
    duration(duration) {}

void Music_Fade_Command::execute() {
    float alpha = calculate_alpha(game.ticks(), start_time, duration);
    music.set_volume(lerp(old_volume, new_volume, alpha));
}

bool Music_Fade_Command::is_complete() const {
    return game.ticks() - start_time > duration;
}



Shake_Screen_Command::Shake_Screen_Command(Game& game, float strength, float speed, long duration) :
        game(game),
        start_time(game.ticks()),
        duration(duration) {
    game.get_camera()->start_shaking(strength, speed);
}

bool Shake_Screen_Command::is_complete() const {
    bool complete = game.ticks() - start_time > duration;
    if (complete)
        game.get_camera()->cease_shaking();
    return complete;
}

Text_Command::Text_Command(Game& game, Map_Object* object, const std::string& text) :
        game(game), object(object), text(text), complete(false) {
    init();
}

Text_Command::Text_Command(Game& game, Map_Object* object, std::vector<std::string> choices, const std::string& text) :
     game(game), object(object), choices(choices), text(text), complete(false)
{
    init();
}

void Text_Command::init() {
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
    float char_width = 5.5;
    float char_height = 8;
    float text_width = char_width * longest_line.length();
    float text_height = char_height * text_lines.size();
    xd::vec2 object_pos = object->get_position() + xd::vec2(16, 0) -
        game.get_camera()->get_position();
    xd::vec2 position(object_pos.x - text_width / 2, object_pos.y - text_height);
    // Make sure text fits on the screen
    if (position.x + text_width > Game::game_width - 10)
        position.x = static_cast<float>(Game::game_width - text_width - char_width * 2);
    if (position.x < 10.0f)
        position.x = 10.0f;
    if (position.y + text_height > Game::game_height - 10)
        position.y = static_cast<float>(Game::game_height - text_height * 2);
    if (position.y < 25.0f)
        position.y = 25.0f;
    // Create the text canvas and show it
    canvas = std::make_shared<Canvas>(game, position, full);
    game.get_map()->get_canvases().push_back(canvas);
    was_disabled = game.get_player()->is_disabled();
    game.get_player()->set_disabled(true);
    canvas->set_visible(true);
}

void Text_Command::execute() {
    if (complete)
        return;
    static std::string action_button =
        Configurations::get<std::string>("controls.action-button");
    complete = game.triggered_once(action_button);
    if (!choices.empty()) {
        if (complete)
            selected_choice = current_choice;
        else
            update_choice();
    }
}

void Text_Command::update_choice() {
    unsigned int old_choice = current_choice;
    if (game.triggered("down"))
        current_choice = (current_choice + 1) % choices.size();
    if (game.triggered("up"))
        current_choice = (current_choice + choices.size() - 1) % choices.size();
    if (old_choice != current_choice)
        canvas->set_text(full_text());
    
}

bool Text_Command::is_complete() const {
    if (complete && canvas->is_visible()) {
        canvas->set_visible(false);
        game.get_player()->set_disabled(was_disabled);
        auto& cvs = game.get_map()->get_canvases();
        cvs.erase(std::remove(cvs.begin(), cvs.end(), canvas), cvs.end());
    }
    return complete;
}

std::string Text_Command::full_text() const {
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
            } else
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

Wait_Command::Wait_Command(Game& game, long duration, int start)
    : game(game), duration(duration),
    start_time(start < 0 ? game.ticks() : start) {}

bool Wait_Command::is_complete() const {
    return is_complete(game.ticks());
}

bool Wait_Command::is_complete(int ticks) const {
    return ticks > start_time + duration;
}
