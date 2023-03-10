#include "../include/audio_player.hpp"
#include "../include/map.hpp"
#include "../include/log.hpp"
#include "../include/configurations.hpp"
#include "../include/utility/string.hpp"
#include "../include/xd/audio.hpp"
#include <stdexcept>

Audio_Player::Audio_Player(std::shared_ptr<xd::audio> audio) : audio(audio), music_was_paused(false) {
    // Set default volumes
    set_global_music_volume(Configurations::get<float>("audio.music-volume"));
    set_global_sound_volume(Configurations::get<float>("audio.sound-volume"));
    // Cache default sounds
    load_global_config_sound("audio.choice-select-sfx", 3, false);
    load_global_config_sound("audio.choice-confirm-sfx", 3, false);
    load_global_config_sound("audio.choice-select-sfx", 3, false);
}

std::shared_ptr<xd::sound> Audio_Player::load_global_config_sound(const std::string& config_name,
        unsigned int channel_count, bool pausable) {
    auto filename = Configurations::get<std::string>(config_name);
    return load_sound(global_cache, config_name, filename, channel_count, pausable);
}

std::shared_ptr<xd::sound> Audio_Player::load_map_sound(Map& current_map,
        const std::string& filename, unsigned int channel_count, bool pausable) {
	return load_sound(current_map.get_audio_cache(), filename, filename, channel_count, pausable);
}

std::shared_ptr<xd::sound> Audio_Player::load_map_config_sound(Map& current_map,
        const std::string& config_name, unsigned int channel_count, bool pausable) {
	auto filename = Configurations::get<std::string>(config_name);
	return load_sound(current_map.get_audio_cache(), config_name, filename, channel_count, pausable);
}

std::shared_ptr<xd::music> Audio_Player::load_music(Map& current_map, const std::string& filename) {
	return load_music(current_map.get_audio_cache(), filename);
}

void Audio_Player::load_map_audio(Map& map) {
    load_audio_from_map_prop(map, "cached-music", true);
    load_audio_from_map_prop(map, "cached-sounds", false);
}

std::shared_ptr<xd::music> Audio_Player::play_music(Map& current_map, float volume) {
    return play_music(current_map, current_map.get_bg_music_filename(), volume);
}

std::shared_ptr<xd::music> Audio_Player::play_music(Map& current_map, const std::string& filename, bool looping, float volume) {
    if (!audio || filename.empty()) return nullptr;

    auto playing_music{ music ? music->get_filename() : "" };
    if (filename == playing_music) return music;

    if (filename == "false") {
        music->stop();
        music.reset();
        return nullptr;
    }

    std::shared_ptr<xd::music> music;
    auto& map_cache = current_map.get_audio_cache();
    if (map_cache.find(filename) != map_cache.end()) {
        music = load_music(current_map, filename);
        LOGGER_D << "Playing cached music file: " << filename;
    } else {
        music = std::make_shared<xd::music>(*audio, filename);
        LOGGER_D << "Playing non-cached music file: " << filename;
    }

    music->set_volume(volume);

    return play_music(current_map, music, looping);
}

std::shared_ptr<xd::music> Audio_Player::play_music(Map& current_map, const std::shared_ptr<xd::music>& new_music, bool looping) {
    if (!audio) return nullptr;

    if (music) {
        music->stop();
    }

    set_playing_music(new_music);
    music->set_looping(looping);
    music->play();

    return music;
}

float Audio_Player::get_global_music_volume() const {
    if (!audio) return 0.0f;
    return audio->get_music_volume();
}

void Audio_Player::set_global_music_volume(float volume) const {
    if (!audio) return;
    audio->set_music_volume(volume);
}

float Audio_Player::get_global_sound_volume() const {
    if (!audio) return 0.0f;
    return audio->get_sound_volume();
}

void Audio_Player::set_global_sound_volume(float volume) const {
    if (!audio) return;
    audio->set_sound_volume(volume);
}

void Audio_Player::update() {
    if (!audio) return;
    audio->update();
}

void Audio_Player::pause() {
    if (!audio) return;

    if (music && Configurations::get<bool>("audio.mute-on-pause")) {
        music_was_paused = music->paused();
        if (!music_was_paused)
            music->pause();
    }

    audio->pause_sounds();
}

void Audio_Player::resume() {
    if (!audio) return;

    if (music && music->paused() && !music_was_paused) {
        music->play();
    }

    audio->resume_sounds();
}

std::shared_ptr<xd::sound> Audio_Player::load_sound(Audio_Cache& cache, const std::string& key,
        const std::string& filename, unsigned int channel_count, bool pausable) {
    if (!audio || filename.empty() || channel_count == 0) return nullptr;

    auto& sounds = cache[key];

    auto old_count = sounds.size();
    if (channel_count > old_count) {
        for (unsigned int i = 0; i < channel_count - old_count; ++i) {
            sounds.emplace_back(std::make_shared<xd::sound>(*audio, filename, get_sound_group_type(pausable)));
        }
    }

    return sounds.back();
}

std::shared_ptr<xd::music> Audio_Player::load_music(Audio_Cache& cache, const std::string& filename) {
    if (!audio || filename.empty()) return nullptr;

    auto& sounds = cache[filename];
    if (sounds.empty()) {
        LOGGER_D << "Loading music file: " << filename;
        sounds.emplace_back(std::make_shared<xd::music>(*audio, filename));
    }

    return std::dynamic_pointer_cast<xd::music>(sounds.back());
}

void Audio_Player::load_audio_from_map_prop(Map& current_map, const std::string& prop_name, bool is_music) {
    auto prop = current_map.get_property(prop_name);
    if (!audio || prop.empty()) return;

    auto& cache = current_map.get_audio_cache();

    auto files = string_utilities::split(prop, ",");
    for (const auto& file : files) {
        auto filename = string_utilities::trim(file);
        if (is_music) {
            load_music(cache, filename);
            continue;
        }

        auto channel_count = 1;
        if (filename.find(":")) {
            auto parts = string_utilities::split(filename, ":");
            if (parts.size() != 2) {
                throw std::runtime_error{"Invalid map property " + prop_name + ": " + prop};
            }
            filename = parts[0];
            channel_count = std::stoi(parts[1]);
        }

        load_sound(cache, filename, filename, channel_count);
    }

    LOGGER_D << "Loaded " << files.size() << " files for the " << prop_name << " map property";
}