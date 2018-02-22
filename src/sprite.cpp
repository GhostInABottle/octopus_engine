#include "../include/sprite.hpp"
#include "../include/sprite_data.hpp"
#include "../include/map_object.hpp"
#include "../include/object_layer.hpp"
#include "../include/object_layer_renderer.hpp"
#include "../include/game.hpp"
#include "../include/utility.hpp"
#include <xd/audio.hpp>
#include <algorithm>
#include <iostream>

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
    long int old_time;
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
    // Last frame where sound was played
    int last_sound_frame;
    // Animation speed modifier (based on object speed)
    float speed;
    // Maximum possible speed modifier
    const static float max_speed;

    Impl(Game& game, std::unique_ptr<Sprite_Data> data) :
        game(game), data(std::move(data))
    {
        pose = &this->data->poses[0];
        frame_count = 0;
        speed = 1.0f;
    }

    void update() {
        if (frame_count == 0 || finished)
            return;

        Frame* current_frame = &pose->frames[frame_index];
        int frame_time = current_frame->duration == -1 ?
                            pose->duration : current_frame->duration;

        frame_time = static_cast<int>(frame_time * speed);

        // If the number of repeats is reached then animation is finished
        if (pose->repeats != -1 && repeat_count >= pose->repeats) {
            // Make sure the last frame is complete before finishing
            if (game.ticks() - old_time > frame_time)
                finished = true;
            return;
        }

        // If animation is still not finished...
        if (!current_frame->sound_file.empty() && last_sound_frame != frame_index) {
            auto sound = game.load_sound(current_frame->sound_file);
            sound->play();
            last_sound_frame = frame_index;
        }
        if (game.ticks() - old_time > frame_time) {
            old_time = game.ticks();
            if (tweening)
                tweening = false;
            frame_index++;
            if (frame_index >= frame_count - 1) {
                repeat_count++;
                last_sound_frame = -1;
            }
            frame_index %= frame_count;
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
            const float time_diff = static_cast<float>(game.ticks() - old_time);
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
        if (pose)
            frame_count = pose->frames.size();
        finished = false;
        tweening = false;
    }

    std::string get_filename() const {
        return data->filename;
    }

    void set_pose(const std::unordered_map<std::string, std::string>& new_tags) {
        // Clear existing tags if needed
        tags.clear();
        // Update current pose tags
        std::string tag_string;
        for (auto new_tag = new_tags.begin(); new_tag != new_tags.end(); ++new_tag) {
            std::string key = capitalize(new_tag->first);
            std::string value = capitalize(new_tag->second);
            this->tags[key] = value;
            tag_string += key + ":" + value + " ";
        }
        int matched_pose = -1;
        // Lookup pose in cache
        if (tag_map.find(tag_string) != tag_map.end()) {
            matched_pose = tag_map[tag_string];
        } else {
            // Map of pose IDs to their tag match count
            std::unordered_map<int, int> matches;
            // Loop over tags incrementing poses that match
            for (auto tag = tags.begin(); tag != tags.end(); ++tag) {
                for (unsigned int i = 0; i < data->poses.size(); ++i) {
                    auto& pose_tags = data->poses[i].tags;
                    if (pose_tags.find(tag->first) != pose_tags.end() &&
                            pose_tags[tag->first] == tag->second) {
                        matches[i]++;
                        // Update best match
                        if (matched_pose == -1 ||
                                matches[i] > matches[matched_pose] ||
                                (matches[i] == matches[matched_pose] &&
                                tag->first == "NAME")) {
                            matched_pose = i;
                            // If all tags are matched, exit loops
                            if (matches[i] == tags.size())
                                break;
                        }
                    }
                }
                if (matches[matched_pose] == tags.size())
                    break;
            }
            // If no tags, use first pose
            if (matched_pose == -1)
                matched_pose = 0;
            // Update pose cache
            tag_map[tag_string] = matched_pose;
        }
        // Set matched pose and reset the sprite
        if (pose != &data->poses[matched_pose] || finished) {
            pose = &data->poses[matched_pose];
            reset();
        }
    }
};

const xd::vec4 Sprite::Impl::default_color(1, 1, 1, 1);
const float Sprite::Impl::max_speed = 10.0f;

Sprite::Sprite(Game& game, std::unique_ptr<Sprite_Data> data)
        : pimpl(new Impl(game, std::move(data))) {
    pimpl->reset();
}

void Sprite::render(Map_Object& object) {
    if (!object.is_visible())
        return;
    auto layer =  object.get_layer();
    auto& batch = layer->renderer->get_batch();
    render(batch, object.get_position(),
           layer->opacity * object.get_opacity(),
           layer->color * object.get_color());
}

void Sprite::render(xd::sprite_batch& batch, xd::vec2 pos, float opacity,
        xd::vec4 color, bool repeat, xd::vec2 repeat_pos) {
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
    batch.add(image, src, pos.x, pos.y, angle,
        frame.magnification, color, get_pose().origin);
}

void Sprite::update(Map_Object& object) {
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
