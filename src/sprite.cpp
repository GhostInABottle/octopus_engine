#include "../include/sprite.hpp"
#include "../include/sprite_data.hpp"
#include "../include/map_object.hpp"
#include "../include/object_layer.hpp"
#include "../include/object_layer_renderer.hpp"
#include "../include/game.hpp"
#include "../include/utility/math.hpp"
#include "../include/utility/string.hpp"
#include "../include/xd/audio.hpp"
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace detail {
    xd::vec4 default_color(1, 1, 1, 1);
}

struct Sprite::Impl {
    // Game instance
    Game& game;
    // Sprite data (poses, frames, etc.)
    std::unique_ptr<Sprite_Data> data;
    // Current tags
    std::unordered_map<std::string, std::string> tags;
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
    // List of playing sounds
    std::vector<std::shared_ptr<xd::sound>> playing_sounds;

    Impl(Game& game, std::unique_ptr<Sprite_Data> data) :
        game(game),
        data(std::move(data)),
        frame_index(0),
        old_time(game.ticks()),
        repeat_count(0),
        frame_count(0),
        tweening(false),
        finished(false),
        paused(false),
        pause_start(-1),
        last_sound_frame(-1),
        speed(1.0f)
    {
        set_default_pose();
    }

    void update() {
        // Remove finished sounds
        if (!playing_sounds.empty()) {
            auto removed = std::remove_if(playing_sounds.begin(), playing_sounds.end(),
                [](const std::shared_ptr<xd::sound>& s) { return s->stopped(); });
            playing_sounds.erase(removed, playing_sounds.end());
        }

        if (frame_count == 0 || finished)
            return;

        Frame* current_frame = &pose->frames[frame_index];
        int frame_time = get_frame_time(*current_frame);

        auto audio = game.get_audio();
        if (audio && !current_frame->sound_file.empty() && last_sound_frame != frame_index) {
            auto sound = std::make_shared<xd::sound>(*audio, current_frame->sound_file);
            sound->play();
            playing_sounds.push_back(sound);
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
        int frame_time = frame.duration == -1 ? pose->duration : frame.duration;
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

    void set_pose(const std::unordered_map<std::string, std::string>& new_tags) {
        // Update current pose tags
        tags = new_tags;
        // Lookup pose in cache
        std::string tag_string;
        for (auto& [key, value] : tags) {
            tag_string += key + ":" + value + " ";
        }
        int matched_pose = -1;
        bool name_matched = false;
        bool default_name_matched = false;
        if (tag_map.find(tag_string) != tag_map.end()) {
            matched_pose = tag_map[tag_string];
        } else {
            // Map of pose IDs to their tag match count
            std::unordered_map<int, unsigned int> matches;
            int default_pose = -1;
            // Loop over tags incrementing poses that match
            for (auto& [key, value] : tags) {
                for (unsigned int i = 0; i < data->poses.size(); ++i) {
                    auto& pose_tags = data->poses[i].tags;
                    if (pose_tags.find(key) == pose_tags.end() || pose_tags[key] != value) continue;
                    matches[i]++;
                    auto& pose_name = pose_tags["NAME"];
                    // Update best default pose
                    if (data->default_pose != "" && pose_name == data->default_pose
                            && compare_matches(i, default_pose, matches) > 0) {
                        default_pose = i;
                        if (pose_name == tags["NAME"]) {
                            default_name_matched = true;
                        }
                    }
                    // Update best match
                    auto comparison = compare_matches(i, matched_pose, matches);
                    if (comparison > 0 || (comparison == 0 && pose_name == tags["NAME"])) {
                        matched_pose = i;
                        // If all tags are matched, exit loops
                        if (matches[i] == tags.size())
                            break;
                    }
                }
                if (matches[matched_pose] == tags.size())
                    break;
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
            if (p.tags.find("NAME") != p.tags.end() && p.tags["NAME"] == data->default_pose) {
                pose = &p;
                return;
            }
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
    if (!object.is_visible())
        return;
    auto layer =  object.get_layer();
    auto& batch = layer->renderer->get_batch();
    render(batch, object.get_position(),
           layer->opacity * object.get_opacity(),
           object.get_magnification(),
           layer->color * object.get_color());
}

void Sprite::render(xd::sprite_batch& batch, xd::vec2 pos, float opacity,
        xd::vec2 mag, xd::vec4 color, bool repeat, xd::vec2 repeat_pos) {
    auto& pose = get_pose();
    auto& frame = get_frame();
    auto& image = frame.image ? frame.image :
        (pose.image ? pose.image : pimpl->data->image);
    if (!image)
        return;
    xd::rect src = frame.rectangle;
    if (repeat) {
        // Sprite's src rectangle position is ignored
        src.x = -repeat_pos.x;
        src.y = -repeat_pos.y;
    }
    float angle = xd::radians(static_cast<float>(frame.angle));
    color.a *= opacity * frame.opacity;
    auto magnification = frame.magnification * mag;
    batch.add(image, src, pos.x, pos.y, angle,
        magnification, color, get_pose().origin);
}

void Sprite::update(Map_Object&) {
   update();
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

void Sprite::set_pose(const std::unordered_map<std::string, std::string>& new_tags) {
    pimpl->set_pose(new_tags);
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
        auto& frame = pose.frames[0];
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
