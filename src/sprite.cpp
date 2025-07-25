#include "sprite.hpp"
#include "sprite_data.hpp"
#include "map/map_object.hpp"
#include "canvas/sprite_canvas.hpp"
#include "map/layers/object_layer.hpp"
#include "map/layers/object_layer_renderer.hpp"
#include "map/layers/image_layer.hpp"
#include "game.hpp"
#include "audio_player.hpp"
#include "configurations.hpp"
#include "utility/math.hpp"
#include "utility/string.hpp"
#include "utility/direction.hpp"
#include "xd/audio.hpp"
#include <algorithm>
#include <cstdlib>
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
    std::shared_ptr<Sprite_Data> data;
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
    // Calculated duration of the current frame
    int frame_duration;
    // Frame markers seen so far
    std::vector<std::string> passed_markers;
    // Number of times the animation repeated so far
    int repeat_count;
    // Total number of frames
    int frame_count;
    // Is animation in a tween frame
    bool tweening;
    // Default color
    const static xd::vec4 default_color;
    // Is the pose completed
    bool completed;
    // Index at which the sprite should complete
    std::vector<int> completion_indexes;
    // If true, the pose won't be updated anymore
    bool stop_updating;
    // Is the sprite paused
    bool paused;
    // Time when sprite got paused
    long pause_start;
    // Last frame where sound was played
    int last_sound_frame;
    // Animation speed modifier
    float speed;
    // Volume of the sprite's sound effects
    float sfx_volume;
    // How fast sound volume falls off
    float sound_attenuation_factor;

    Impl(Game& game, std::shared_ptr<Sprite_Data> data) :
        game(game),
        data(data),
        current_pose_direction(Direction::NONE),
        frame_index(0),
        old_time(game.ticks()),
        frame_duration(-1),
        repeat_count(0),
        frame_count(0),
        tweening(false),
        completed(false),
        stop_updating(false),
        paused(false),
        pause_start(-1),
        last_sound_frame(-1),
        speed(1.0f),
        sfx_volume(1.0f),
        sound_attenuation_factor(Configurations::get<float>("audio.sound-attenuation-factor")) {
        set_default_pose();
    }

    void render(xd::sprite_batch& batch, xd::vec2 pos, float opacity = 1.0f,
            xd::vec2 mag = xd::vec2(1.0f), xd::vec4 color = xd::vec4(1.0f),
            std::optional<float> angle = std::nullopt, std::optional<xd::vec2> origin = std::nullopt,
        bool repeat = false, xd::vec2 repeat_pos = xd::vec2()) const {
        auto& frame = pose->frames[frame_index];
        auto& image = frame.image ? frame.image :
            (pose->image ? pose->image : data->image);
        if (!image) return;

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

    void update(std::optional<xd::vec2> object_pos = std::nullopt) {
        if (frame_count == 0 || stop_updating) return;

        auto current_frame = &pose->frames[frame_index];

        if (frame_duration < 0) {
            frame_duration = get_frame_time(*current_frame);
        }

        auto audio = game.get_audio_player().get_audio();
        auto& sound_file = current_frame->sound_file;
        auto play_sfx = audio && sound_file
            && (last_sound_frame != frame_index || sound_file->stopped());
        if (play_sfx) {
            if (object_pos) {
                update_sound_attenuation(*object_pos, true);
            } else {
                sound_file->set_volume(current_frame->sound_volume * sfx_volume);
            }
            sound_file->play();
            last_sound_frame = frame_index;
        }

        if (get_passed_time() > frame_duration) {
            auto complete_infinite = !completed
                && pose->require_completion
                && std::find(completion_indexes.begin(), completion_indexes.end(),
                    frame_index) != completion_indexes.end();
            if (complete_infinite) {
                // When a pose requires completion, mark it as complete for 1 frame
                completed = true;
                return;
            }

            old_time = game.ticks();
            tweening = false;

            if (frame_index + 1 >= frame_count) {
                repeat_count++;
                last_sound_frame = -1;
                if (finished_repeating()) {
                    completed = true;
                    stop_updating = true;
                    return;
                } else if (!passed_markers.empty()) {
                    passed_markers.clear();
                }
            }
            frame_index = (frame_index + 1) % frame_count;
            frame_duration = -1;

            if (completed && !completion_indexes.empty()) {
                completed = false;
            }
        }

        current_frame = &pose->frames[frame_index];
        if (!current_frame->marker.empty()) {
            passed_markers.push_back(current_frame->marker);
        }

        if (!tweening && current_frame->tween_frame) {
            Frame& prev_frame = pose->frames[frame_index - 1];
            current_frame->rectangle.x = prev_frame.rectangle.x;
            current_frame->rectangle.y = prev_frame.rectangle.y;
            current_frame->rectangle.w = prev_frame.rectangle.w;
            current_frame->rectangle.h = prev_frame.rectangle.h;
            old_time = game.ticks();
            frame_duration = get_frame_time(*current_frame);
            tweening = true;
        }

        if (tweening) {
            Frame& prev_frame = pose->frames[frame_index - 1];
            Frame& next_frame = pose->frames[frame_index + 1];
            const float time_diff = static_cast<float>(get_passed_time());
            float alpha = time_diff / frame_duration;
            alpha = std::min(std::max(alpha, 0.0f), 1.0f);
            current_frame->magnification = lerp(prev_frame.magnification,
                next_frame.magnification, alpha);
            current_frame->angle = static_cast<int>(
                lerp(static_cast<float>(prev_frame.angle),
                    static_cast<float>(next_frame.angle), alpha));
            current_frame->opacity =
                lerp(prev_frame.opacity, next_frame.opacity, alpha);
        }
    }

    void reset(bool reset_current_frame) {
        frame_duration = -1;
        repeat_count = 0;
        last_sound_frame = -1;
        completed = false;
        completion_indexes.clear();
        passed_markers.clear();
        stop_updating = false;
        tweening = false;
        if (!pose || reset_current_frame) {
            frame_index = 0;
            old_time = game.ticks();
        }

        if (!pose) return;

        frame_count = pose->frames.size();
        if (!reset_current_frame) {
            frame_index = frame_index % frame_count;
        }

        if (!pose->require_completion) return;

        if (pose->completion_frames) {
            completion_indexes = *pose->completion_frames;
        } else if (frame_count > 0) {
            completion_indexes.push_back(frame_count - 1);
        } else {
            completed = true;
        }
    }

    int get_frame_time(const Frame& frame) const {
        int frame_time = frame.duration == -1
            ? pose->duration
            : frame.duration;
        if (frame.max_duration > frame_time) {
            frame_time += std::rand() % (frame.max_duration - frame_time + 1);
        }
        return static_cast<int>(frame_time / speed);
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

    long get_paused_time() const {
        if (pause_start == -1) return 0;
        return game.ticks() - pause_start;
    }

    long get_passed_time() const {
        return game.ticks() - old_time - get_paused_time();
    }

    std::optional<std::string> get_last_marker() const {
        if (passed_markers.empty()) return std::nullopt;

        return passed_markers.back();
    }

    bool passed_marker(const std::string& marker) const {
        for (auto& m : passed_markers) {
            if (m == marker) return true;
        }

        return false;
    }

    void set_pose(const std::string& pose_name, const std::string& state_name,
            Direction dir, bool reset_current_frame) {
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
            unsigned int matches_needed = (pose_name.empty() ? 0 : 1)
                + (state_name.empty() ? 0 : 1)
                + (dir == Direction::NONE ? 0 : 1);
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
        if (pose != &data->poses[matched_pose] || stop_updating) {
            pose = &data->poses[matched_pose];
            reset(reset_current_frame);
        }
    }

    // A pose is better than current best if there's no best yet or if it matches more tags
    int compare_matches(int candidate_index, int best_index,
            std::unordered_map<int, unsigned int>& matches) {
        auto candidate = matches[candidate_index];
        auto best = best_index > -1 ? matches[best_index] : 0;
        return candidate == best ? 0
            : (candidate > best ? 1 : -1);
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

    void update_sound_attenuation(xd::vec2 object_pos, bool force = false) {
        auto current_frame = &pose->frames[frame_index];
        auto& sound_file = current_frame->sound_file;
        auto skip = !sound_file || (!force && !sound_file->playing());
        if (skip) return;

        auto original_volume = current_frame->sound_volume;
        const auto player = game.get_player();
        auto distance = xd::length(object_pos - player->get_centered_position());

        // Volume is at original level within [factor] pixels,
        // then falls off based on distance
        auto volume = std::min(original_volume,
            original_volume * sound_attenuation_factor / distance);
        sound_file->set_volume(volume * sfx_volume);
    }
};

const xd::vec4 Sprite::Impl::default_color(1, 1, 1, 1);

Sprite::Sprite(Game& game, std::shared_ptr<Sprite_Data> data)
        : pimpl(std::make_unique<Impl>(game, data)) {
    pimpl->reset(true);
}

Sprite::~Sprite() {}

void Sprite::render(Map_Object& object) {
    if (!object.is_visible()) return;

    auto layer = object.get_layer();
    auto& batch = layer->get_renderer()->get_batch();
    auto color = object.uses_layer_color()
        ? object.get_color() * layer->get_color()
        : object.get_color();

    pimpl->render(batch, object.get_position(),
        layer->get_opacity() * object.get_opacity(),
        object.get_magnification(),
        color);
}

void Sprite::render(xd::sprite_batch& batch, const Sprite_Canvas& canvas, const xd::vec2 pos) const {
    if (!canvas.is_visible()) return;

    pimpl->render(batch,
        pos,
        canvas.get_opacity(),
        canvas.get_magnification(),
        canvas.get_color(),
        canvas.get_angle(),
        canvas.get_origin());
}

void Sprite::render(xd::sprite_batch& batch, const Image_Layer& image_layer, const xd::vec2 pos) const {
    if (!image_layer.is_visible()) return;

    pimpl->render(batch,
        pos,
        image_layer.get_opacity(),
        xd::vec2(1.0f),             // magnification
        image_layer.get_color(),    // color
        std::nullopt,               // angle
        std::nullopt,               // origin
        image_layer.is_repeating(),
        image_layer.get_position());
}

void Sprite::update(Map_Object& object) {
    if (!object.is_visible()) return;

    if (!object.is_sound_attenuation_enabled()) {
        pimpl->update();
        return;
    }

    auto pos = object.get_centered_position();
    pimpl->update_sound_attenuation(pos);
    pimpl->update(pos);
}

void Sprite::update() {
    pimpl->update();
}

void Sprite::reset(bool reset_current_frame) {
    pimpl->reset(reset_current_frame);
}

std::string Sprite::get_filename() const {
    return pimpl->get_filename();
}

void Sprite::set_pose(const std::string& pose_name, const std::string& state_name,
        Direction dir, bool reset_current_frame) {
    pimpl->set_pose(pose_name, state_name, dir, reset_current_frame);
}

Pose& Sprite::get_pose() {
    return *pimpl->pose;
}

const Pose& Sprite::get_pose() const {
    return *pimpl->pose;
}

xd::rect Sprite::get_bounding_box() const {
    return pimpl->pose->bounding_box;
}

std::optional<xd::circle> Sprite::get_bounding_circle() const {
    return pimpl->pose->bounding_circle;
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

int Sprite::get_frame_index() const {
    return pimpl->frame_index;
}

std::optional<std::string> Sprite::get_last_marker() const {
    return pimpl->get_last_marker();
}

bool Sprite::passed_marker(const std::string& marker) const {
    return pimpl->passed_marker(marker);
}

bool Sprite::is_complete() const {
    return pimpl->completed;
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

float Sprite::get_speed() const {
    return pimpl->speed;
}

void Sprite::set_speed(float speed) {
    pimpl->speed = speed;
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
