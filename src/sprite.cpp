#include "../include/sprite.hpp"
#include "../include/sprite_data.hpp"
#include "../include/map_object.hpp"
#include "../include/canvas/sprite_canvas.hpp"
#include "../include/object_layer.hpp"
#include "../include/object_layer_renderer.hpp"
#include "../include/image_layer.hpp"
#include "../include/game.hpp"
#include "../include/audio_player.hpp"
#include "../include/configurations.hpp"
#include "../include/utility/math.hpp"
#include "../include/utility/string.hpp"
#include "../include/utility/direction.hpp"
#include "../include/xd/audio.hpp"
#include "../include/xd/graphics.hpp"
#include <algorithm>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <vector>

namespace detail {
    static xd::vec4 default_color(1, 1, 1, 1);
}

struct Sprite::Impl {
    // Game instance
    Game& game;
    // Sprite data (poses, frames, etc.)
    std::unique_ptr<Sprite_Data> data;
    // Current pose name
    std::string current_pose_name;
    // Current pose state
    std::string current_pose_state;
    // Current direction
    Direction current_pose_direction;
    // Cache of tag combination to poses
    std::unordered_map<std::string, int> tag_map;
    // Currently active pose
    Pose* pose;
    // Source rectangle of current frame
    xd::rect src;
    // Current frame index
    int frame_index;
    // Used to check if enough time has passed since last frame
    long old_time;
    // Number of times the animation repeated so far
    int repeat_count;
    // Total number of frames
    int frame_count;
    // Is animation in a tween frame
    bool tweening;
    // Default color
    const static xd::vec4 default_color;
    // Is the pose finished
    bool finished;
    // Is the sprite paused
    bool paused;
    // Time when sprite got paused
    long pause_start;
    // Last frame where sound was played
    int last_sound_frame;
    // Animation speed modifier (based on object speed)
    float speed;
    // Maximum possible speed modifier
    const static float max_speed;
    // Volume of the sprite's sound effects
    float sfx_volume;
    // How fast sound volume falls off
    float sound_attenuation_factor;

    Impl(Game& game, std::unique_ptr<Sprite_Data> data) :
        game(game),
        data(std::move(data)),
        current_pose_direction(Direction::NONE),
        frame_index(0),
        old_time(game.ticks()),
        repeat_count(0),
        frame_count(0),
        tweening(false),
        finished(false),
        paused(false),
        pause_start(-1),
        last_sound_frame(-1),
        speed(1.0f),
        sfx_volume(1.0f),
        sound_attenuation_factor(Configurations::get<float>("audio.sound-attenuation-factor"))
    {
        set_default_pose();
    }

    void render(xd::sprite_batch& batch, xd::vec2 pos, float opacity = 1.0f,
            xd::vec2 mag = xd::vec2(1.0f), xd::vec4 color = xd::vec4(1.0f),
            std::optional<float> angle = std::nullopt, std::optional<xd::vec2> origin = std::nullopt,
        bool repeat = false, xd::vec2 repeat_pos = xd::vec2()) {
        auto& frame = pose->frames[frame_index];
        auto& image = frame.image ? frame.image :
            (pose->image ? pose->image : data->image);
        if (!image)
            return;
        xd::rect src = frame.rectangle;
        if (repeat) {
            // Sprite's src rectangle position is ignored
            src.x = -repeat_pos.x;
            src.y = -repeat_pos.y;
        }
        color.a *= opacity * frame.opacity;
        batch.add(image, src,
            pos.x, pos.y,
            xd::radians(angle.value_or(static_cast<float>(frame.angle))),
            frame.magnification * mag,
            color,
            origin.value_or(pose->origin));
    }

    void update() {
        if (frame_count == 0 || finished)
            return;

        auto current_frame = &pose->frames[frame_index];
        int frame_time = get_frame_time(*current_frame);

        auto audio = game.get_audio_player().get_audio();
        auto sound_file = current_frame->sound_file.get();
        auto play_sfx = audio && sound_file
            && (last_sound_frame != frame_index || sound_file->stopped());
        if (play_sfx) {
            sound_file->set_volume(current_frame->sound_volume * sfx_volume);
            sound_file->play();
            last_sound_frame = frame_index;
        }

        if (passed_time() > frame_time) {
            old_time = game.ticks();
            if (tweening) tweening = false;

            if (frame_index + 1 >= frame_count) {
                repeat_count++;
                last_sound_frame = -1;
                if (finished_repeating()) {
                    finished = true;
                    return;
                }
            }
            frame_index = (frame_index + 1) % frame_count;
        }

        current_frame = &pose->frames[frame_index];
        if (!tweening && current_frame->tween_frame) {
            Frame& prev_frame = pose->frames[frame_index - 1];
            current_frame->rectangle.x = prev_frame.rectangle.x;
            current_frame->rectangle.y = prev_frame.rectangle.y;
            current_frame->rectangle.w = prev_frame.rectangle.w;
            current_frame->rectangle.h = prev_frame.rectangle.h;
            old_time = game.ticks();
            tweening = true;
        }
        if (tweening) {
            Frame& prev_frame = pose->frames[frame_index - 1];
            Frame& next_frame = pose->frames[frame_index + 1];
            const float time_diff = static_cast<float>(passed_time());
            float alpha = time_diff / frame_time;
            alpha = std::min(std::max(alpha, 0.0f), 1.0f);
            current_frame->magnification = lerp(prev_frame.magnification,
                next_frame.magnification, alpha);
            current_frame->angle= static_cast<int>(
                lerp(static_cast<float>(prev_frame.angle),
                static_cast<float>(next_frame.angle), alpha));
            current_frame->opacity=
                lerp(prev_frame.opacity, next_frame.opacity, alpha);
        }
    }

    void reset() {
        frame_index = 0;
        old_time = game.ticks();
        repeat_count = 0;
        last_sound_frame = -1;
        if (pose)
            frame_count = pose->frames.size();
        finished = false;
        tweening = false;
    }

    int get_frame_time(const Frame& frame) const {
        const int frame_time = frame.duration == -1 ? pose->duration : frame.duration;
        return static_cast<int>(frame_time * speed);
    }

    std::string get_filename() const {
        return data->filename;
    }

    bool finished_repeating() const {
        return pose->repeats != -1 && repeat_count >= pose->repeats;
    }

    void pause() {
        paused = true;
        pause_start = game.ticks();
    }

    void resume() {
        paused = false;
        pause_start = -1;
    }

    long paused_time() const {
        if (pause_start == -1) return 0;
        return game.ticks() - pause_start;
    }

    long passed_time() const {
        return game.ticks() - old_time - paused_time();
    }

    bool is_completed() const {
        return frame_count > 0
            && frame_index == frame_count - 1
            && passed_time() >= get_frame_time(pose->frames[frame_index]);
    }

    void set_pose(const std::string& pose_name, const std::string& state_name, Direction dir) {
        // Update current pose tags
        current_pose_name = string_utilities::capitalize(pose_name);
        current_pose_state = string_utilities::capitalize(state_name);
        current_pose_direction = dir;
        // Lookup pose in cache
        std::string tag_string;
        tag_string = "P:" + current_pose_name + "|S:" + current_pose_state + "|D:" + direction_to_string(dir);
        int matched_pose = -1;
        bool default_name_matched = false;
        if (tag_map.find(tag_string) != tag_map.end()) {
            matched_pose = tag_map[tag_string];
        } else {
            // Map of pose IDs to their tag match count
            std::unordered_map<int, unsigned int> matches;
            int matches_needed = (pose_name.empty() ? 0 : 1) + (state_name.empty() ? 0 : 1) + (dir == Direction::NONE ? 0 : 1);
            int default_pose = -1;
            bool is_default = data->default_pose != "" && current_pose_name == data->default_pose;
            // Loop over poses incrementing poses that match
            for (unsigned int i = 0; i < data->poses.size(); ++i) {
                auto& pose = data->poses[i];
                auto name_matched = current_pose_name == pose.name;
                if (!current_pose_name.empty() && name_matched) {
                    matches[i]++;
                    // Update best default pose
                    if (is_default && compare_matches(i, default_pose, matches) > 0) {
                        default_pose = i;
                        default_name_matched = true;
                    }
                }
                if (!current_pose_state.empty() && current_pose_state == pose.state) {
                    matches[i]++;
                }
                if (dir != Direction::NONE && dir == pose.direction) {
                    matches[i]++;
                }
                // Update best match
                auto comparison = compare_matches(i, matched_pose, matches);
                if (comparison > 0 || (comparison == 0 && name_matched)) {
                    matched_pose = i;
                }
                if (matches[i] == matches_needed) {
                    break;
                }
            }

            // Prefer default pose to other poses with same matches
            if (matched_pose == -1 || (default_name_matched && compare_matches(matched_pose, default_pose, matches) == 0)) {
                matched_pose = default_pose == -1 ? 0 : default_pose;
            }
            // Update pose cache
            tag_map[tag_string] = matched_pose;
        }
        // Set matched pose and reset the sprite
        if (pose != &data->poses[matched_pose] || finished) {
            pose = &data->poses[matched_pose];
            reset();
        }
    }

    // A pose is better than current best if there's no best yet or if it matches more tags
    int compare_matches(int candidate_index, int best_index, 
            std::unordered_map<int, unsigned int>& matches) {
        auto candidate = matches[candidate_index];
        auto best = best_index > -1 ? matches[best_index] : 0;
        if (candidate > best) return 1;
        if (best > candidate) return -1;
        return 0;
    }

    void set_default_pose() {
        if (data->default_pose.empty()) {
            pose = &data->poses[0];
            return;
        }
        for (auto& p : data->poses) {
            if (p.name == data->default_pose) {
                pose = &p;
                return;
            }
        }
    }

    void update_sound_attenuation(Map_Object& object) {
        auto player_position{game.get_player()->get_centered_position()};
        auto distance = xd::length(object.get_centered_position() - player_position);

        // Sound is 1 within [factor] pixels, then falls off based on distance
        sfx_volume = std::min(1.0f, sound_attenuation_factor / (1.0f + distance));
        auto current_frame = &pose->frames[frame_index];
        if (current_frame->sound_file && current_frame->sound_file->playing()) {
            current_frame->sound_file->set_volume(sfx_volume);
        }
    }
};

const xd::vec4 Sprite::Impl::default_color(1, 1, 1, 1);
const float Sprite::Impl::max_speed = 10.0f;

Sprite::Sprite(Game& game, std::unique_ptr<Sprite_Data> data)
        : pimpl(std::make_unique<Impl>(game, std::move(data))) {
    pimpl->reset();
}

Sprite::~Sprite() {}

void Sprite::render(Map_Object& object) {
    if (!object.is_visible()) return;

    auto layer =  object.get_layer();
    auto& batch = layer->renderer->get_batch();
    auto color = object.uses_layer_color()
        ? object.get_color() * layer->tint_color
        : object.get_color();

    pimpl->render(batch, object.get_position(),
           layer->opacity * object.get_opacity(),
           object.get_magnification(),
           color);
}

void Sprite::render(xd::sprite_batch& batch, const Sprite_Canvas& canvas, const xd::vec2 pos) {
    if (!canvas.is_visible()) return;

    pimpl->render(batch,
        pos,
        canvas.get_opacity(),
        canvas.get_magnification(),
        canvas.get_color(),
        canvas.get_angle(),
        canvas.get_origin());
}

void Sprite::render(xd::sprite_batch& batch, const Image_Layer& image_layer, const xd::vec2 pos) {
    if (!image_layer.visible) return;

    pimpl->render(batch,
        pos,
        image_layer.opacity,
        xd::vec2(1.0f),     // magnification
        xd::vec4(1.0f),     // Color
        std::nullopt,       // angle
        std::nullopt,       // origin
        image_layer.repeat,
        image_layer.position);
}

void Sprite::update(Map_Object& object) {
    if (object.is_sound_attenuation_enabled()) {
        pimpl->update_sound_attenuation(object);
    }
    pimpl->update();
}

void Sprite::update() {
    pimpl->update();
}

void Sprite::reset() {
    pimpl->reset();
}

std::string Sprite::get_filename() const {
    return pimpl->get_filename();
}

void Sprite::set_pose(const std::string& pose_name, const std::string& state_name, Direction dir) {
    pimpl->set_pose(pose_name, state_name, dir);
}

Pose& Sprite::get_pose() {
    return *pimpl->pose;
}

xd::rect Sprite::get_bounding_box() const {
    return pimpl->pose->bounding_box;
}

std::string Sprite::get_default_pose() const {
    return pimpl->data->default_pose;
}

xd::vec2 Sprite::get_size() const {
    xd::vec2 size;
    auto& pose = *pimpl->pose;
    if (pose.frames.size() > 0) {
        auto& frame = pose.frames.front();
        size = xd::vec2(frame.rectangle.w, frame.rectangle.h);
    }
    return size;
}

Frame& Sprite::get_frame() {
     return pimpl->pose->frames[pimpl->frame_index];
}

const Frame& Sprite::get_frame() const {
     return pimpl->pose->frames[pimpl->frame_index];
}


bool Sprite::is_stopped() const {
    return pimpl->finished;
}

void Sprite::stop() {
    pimpl->finished = true;
}

bool Sprite::is_paused() const {
    return pimpl->paused;
}

void Sprite::pause() {
    pimpl->pause();
}

void Sprite::resume() {
    pimpl->resume();
}

bool Sprite::is_completed() const {
    return pimpl->is_completed();
}

float Sprite::get_speed() const {
    return pimpl->speed;
}

void Sprite::set_speed(float speed) {
    // Scale sprite speed in the opposite direction of object speed,
    // between 0.5 for max speed (10) and 2 for min speed (0)
    // but also make sure object speed 1 maps to sprite speed 1
    // s-speed = s-min + (s-max - s-min) * (o-speed - o-min) / (o-max - o-min)
    speed = std::max(0.0f, std::min(pimpl->max_speed, speed));
    if (speed <= 1)
        pimpl->speed = 2.0f - speed;
    else
        pimpl->speed = 1.0f - 0.5f * (speed - 1.0f) / (pimpl->max_speed - 1.0f);
}

bool Sprite::is_eight_directional() const {
    return pimpl->data->has_diagonal_directions;
}

float Sprite::get_sfx_volume() const {
    return pimpl->sfx_volume;
}

void Sprite::set_sfx_volume(float volume) {
    pimpl->sfx_volume = volume;
}