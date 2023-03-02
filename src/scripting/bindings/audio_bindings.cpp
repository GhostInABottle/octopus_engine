#include "../../../include/scripting/script_bindings.hpp"
#include "../../../include/scripting/scripting_interface.hpp"
#include "../../../include/game.hpp"
#include "../../../include/command_result.hpp"
#include "../../../include/commands/fade_music_command.hpp"
#include "../../../include/xd/lua.hpp"
#include "../../../include/xd/audio.hpp"
#include <string>
#include <memory>

void bind_audio_types(sol::state& lua, Game& game) {
    // Sound effect
    auto sound = lua.new_usertype<xd::sound>("Sound",
        sol::call_constructor, sol::factories(
            [&](const std::string& filename) {
                auto group_type = game.get_sound_group_type();
                return std::make_unique<xd::sound>(*game.get_audio(), filename, group_type);
            },
            [&](const std::string& filename, bool pausable) {
                auto group_type = pausable
                    ? channel_group_type::sound
                    : channel_group_type::non_pausable_sound;
                return std::make_unique<xd::sound>(*game.get_audio(), filename, group_type);
            }
            ));
    sound["playing"] = sol::property(&xd::sound::playing);
    sound["paused"] = sol::property(&xd::sound::paused);
    sound["stopped"] = sol::property(&xd::sound::stopped);
    sound["offset"] = sol::property(&xd::sound::get_offset, &xd::sound::set_offset);
    sound["volume"] = sol::property(&xd::sound::get_volume, &xd::sound::set_volume);
    sound["pitch"] = sol::property(&xd::sound::get_pitch, &xd::sound::set_pitch);
    sound["looping"] = sol::property(&xd::sound::looping, &xd::sound::set_looping);
    sound["filename"] = sol::property(&xd::sound::get_filename);
    sound["play"] = &xd::sound::play;
    sound["pause"] = &xd::sound::pause;
    sound["stop"] = &xd::sound::stop;
    sound["set_loop_points"] = &xd::sound::set_loop_points;

    // Background music
    auto music = lua.new_usertype<xd::music>("Music",
        sol::call_constructor, sol::factories(
            [&](const std::string& filename) {
                return std::make_shared<xd::music>(*game.get_audio(), filename);
            }
    ));
    music["playing"] = sol::property(&xd::music::playing);
    music["paused"] = sol::property(&xd::music::paused);
    music["stopped"] = sol::property(&xd::music::stopped);
    music["offset"] = sol::property(&xd::music::get_offset, &xd::music::set_offset);
    music["volume"] = sol::property(&xd::music::get_volume, &xd::music::set_volume);
    music["pitch"] = sol::property(&xd::music::get_pitch, &xd::music::set_pitch);
    music["looping"] = sol::property(&xd::music::looping, &xd::music::set_looping);
    music["filename"] = sol::property(&xd::music::get_filename);
    music["play"] = &xd::music::play;
    music["pause"] = &xd::music::pause;
    music["stop"] = &xd::music::stop;
    music["set_loop_points"] = &xd::music::set_loop_points;
    music["fade"] = [&](xd::music* music, float volume, long duration) {
        auto si = game.get_current_scripting_interface();
        return si->register_command<Fade_Music_Command>(game, volume, duration);
    };
}
