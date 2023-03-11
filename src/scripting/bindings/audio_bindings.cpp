#include "../../../include/scripting/script_bindings.hpp"
#include "../../../include/scripting/scripting_interface.hpp"
#include "../../../include/game.hpp"
#include "../../../include/map.hpp"
#include "../../../include/audio_player.hpp"
#include "../../../include/command_result.hpp"
#include "../../../include/commands/fade_music_command.hpp"
#include "../../../include/xd/lua.hpp"
#include "../../../include/xd/audio.hpp"
#include <string>
#include <memory>
#include <optional>

void bind_audio_types(sol::state& lua, Game& game) {
    // Sound effect
    auto sound = lua.new_usertype<xd::sound>("Sound",
        sol::call_constructor, sol::factories(
            [&](const std::string& filename) {
                auto audio = game.get_audio_player().get_audio();
                auto group_type = game.get_sound_group_type();
                return std::make_unique<xd::sound>(*audio, filename, group_type);
            },
            [&](const std::string& filename, bool pausable) {
                auto& audio_player = game.get_audio_player();
                auto group_type = audio_player.get_sound_group_type(pausable);
                return std::make_unique<xd::sound>(*audio_player.get_audio(), filename, group_type);
            }
        )
    );
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
                auto audio = game.get_audio_player().get_audio();
                return std::make_shared<xd::music>(*audio, filename);
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

    // Cached audio player
    auto audio_player = lua.new_usertype<Audio_Player>("Audio_Player");
    audio_player["playing_music"] = sol::property(&Audio_Player::get_playing_music,
        &Audio_Player::set_playing_music);
    audio_player["global_music_volume"] = sol::property(&Audio_Player::get_global_music_volume,
        &Audio_Player::set_global_music_volume);
    audio_player["global_sound_volume"] = sol::property(&Audio_Player::get_global_sound_volume,
        &Audio_Player::set_global_sound_volume);

    audio_player["load_global_sound"] = [](Audio_Player& audio_player, const std::string& filename,
            std::optional<int> channel_count, std::optional<bool> pausable) {
        return audio_player.load_global_sound(filename, channel_count.value_or(1), pausable.value_or(true));
    };
    audio_player["load_global_config_sound"] = [](Audio_Player& audio_player, const std::string& config_name,
            std::optional<int> channel_count, std::optional<bool> pausable) {
        return audio_player.load_global_config_sound(config_name, channel_count.value_or(1), pausable.value_or(true));
    };
    audio_player["load_map_sound"] = [&game](Audio_Player& audio_player, const std::string& filename,
            std::optional<int> channel_count, std::optional<bool> pausable) {
        auto& map = *game.get_map();
        return audio_player.load_map_sound(map, filename, channel_count.value_or(1), pausable.value_or(true));
    };
    audio_player["load_map_config_sound"] = [&game](Audio_Player& audio_player, const std::string& config_name,
            std::optional<int> channel_count, std::optional<bool> pausable) {
        auto& map = *game.get_map();
        return audio_player.load_map_config_sound(map, config_name, channel_count.value_or(1), pausable.value_or(true));
    };

    audio_player["load_music"] = [&game](Audio_Player& audio_player, const std::string& filename) {
        auto& map = *game.get_map();
        return audio_player.load_music(map, filename);
    };

    audio_player["play_music"] = sol::overload(
        [&game](Audio_Player& audio_player, const std::string& filename) {
            return audio_player.play_music(*game.get_map(), filename);
        },
        [&game](Audio_Player& audio_player, const std::string& filename, float volume) {
            return audio_player.play_music(*game.get_map(), filename, true, volume);
        },
        [&game](Audio_Player& audio_player, const std::string& filename, bool looping) {
            return audio_player.play_music(*game.get_map(), filename, looping);
        },
        [&game](Audio_Player& audio_player, const std::string& filename, bool looping, float volume) {
            return audio_player.play_music(*game.get_map(), filename, looping, volume);
        },
        [&game](Audio_Player& audio_player, const std::shared_ptr<xd::music>& music) {
            return audio_player.play_music(*game.get_map(), music);
        },
        [&game](Audio_Player& audio_player, const std::shared_ptr<xd::music>& music, bool looping) {
            return audio_player.play_music(*game.get_map(), music, looping);
        }
    );

    audio_player["play_sound"] = [&game](Audio_Player& audio_player, const std::string& filename,
            std::optional<float> pitch, std::optional<float> volume) {
        return audio_player.play_sound(game, filename, pitch.value_or(1.0f), volume.value_or(1.0f));
    };
    audio_player["play_config_sound"] = [&game](Audio_Player& audio_player, const std::string& config_name,
            std::optional<float> pitch, std::optional<float> volume) {
        return audio_player.play_config_sound(game, config_name, pitch.value_or(1.0f), volume.value_or(1.0f));
    };
}
