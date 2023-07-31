#ifndef HPP_AUDIO_PLAYER
#define HPP_AUDIO_PLAYER

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <optional>
#include "xd/audio/channel_group_type.hpp"

class Game;
class Map;

namespace xd {
    class music;
    class sound;
    class audio;
}

class Audio_Player {
public:
    Audio_Player(std::shared_ptr<xd::audio> audio);
    // Load a sound globally
    std::shared_ptr<xd::sound> load_global_sound(const std::string& filename,
            unsigned int channel_count = 1, bool pausable = true) {
        return load_sound(global_cache, filename, filename, channel_count, pausable);
    }
    // Load sound from config
    std::shared_ptr<xd::sound> load_global_config_sound(const std::string& config_name,
        unsigned int channel_count = 1, bool pausable = true);
    // Load sound for the current map
    std::shared_ptr<xd::sound> load_map_sound(Map& current_map, const std::string& filename,
        unsigned int channel_count = 1, bool pausable = true);
    // Load sound from config and cache it for current map
    std::shared_ptr<xd::sound> load_map_config_sound(Map& current_map, const std::string& config_name,
        unsigned int channel_count = 1, bool pausable = true);
    // Load music for the current map
    std::shared_ptr<xd::music> load_music(Map& current_map, const std::string& filename);
    // Load and cache map audio
    void load_map_audio(Map& map);
    // Get audio system pointer
    xd::audio* get_audio() { return audio.get(); }
    // Get the active channel group for sound effects
    channel_group_type get_sound_group_type(bool pausable) const {
        return pausable
            ? channel_group_type::sound
            : channel_group_type::non_pausable_sound;
    }
    // Get the default audio folder
    const std::string& get_audio_folder() const {
        return audio_folder;
    }
    // Play and set the currently playing music for the map
    std::shared_ptr<xd::music> play_music(Map& current_map, std::optional<float> volume = std::nullopt) {
        return play_music_or_ambient(true, current_map, volume);
    }
    std::shared_ptr<xd::music> play_music(Map& current_map, const std::string& filename,
            bool looping = true, std::optional<float> volume = std::nullopt) {
        return play_music_or_ambient(true, current_map, filename, looping, volume);
    }
    std::shared_ptr<xd::music> play_music(Map& current_map,
            const std::shared_ptr<xd::music>& new_music, bool looping = true) {
        return play_music_or_ambient(true, current_map, new_music, looping);
    }
    // Play and set the currently playing ambient sound for the map
    std::shared_ptr<xd::music> play_ambient(Map& current_map, std::optional<float> volume = std::nullopt) {
        return play_music_or_ambient(false, current_map, volume);
    }
    std::shared_ptr<xd::music> play_ambient(Map& current_map, const std::string& filename,
            bool looping = true, std::optional<float> volume = std::nullopt) {
        return play_music_or_ambient(false, current_map, filename, looping, volume);
    }
    std::shared_ptr<xd::music> play_ambient(Map& current_map,
            const std::shared_ptr<xd::music>& new_ambient, bool looping = true) {
        return play_music_or_ambient(false, current_map, new_ambient, looping);
    }
    // Play a sound effect
    std::shared_ptr<xd::sound> play_sound(Game& game, const std::string& filename,
        float pitch = 1.0f, float volume = 1.0f, std::string key = "");
    std::shared_ptr<xd::sound> play_config_sound(Game& game, const std::string& config_name,
        float pitch = 1.0f, float volume = 1.0f);
    // Get the music currently playing
    std::shared_ptr<xd::music> get_playing_music() { return music; }
    // Get the ambient sound currently playing
    std::shared_ptr<xd::music> get_playing_ambient() { return ambient; }
    // Set the currently playing music
    void set_playing_music(const std::shared_ptr<xd::music>& music) {
        this->music = music;
    }
    // Set the currently playing music
    void set_playing_ambient(const std::shared_ptr<xd::music>& ambient) {
        this->ambient = ambient;
    }
    // Set or get the global volume for music
    float get_global_music_volume() const;
    void set_global_music_volume(float volume) const;
    // Set or get the global volume for sound
    float get_global_sound_volume() const;
    void set_global_sound_volume(float volume) const;
    // Update the audio subsystem
    void update();
    // Pause playing music and sounds
    void pause();
    // Resume playing music and sounds
    void resume();
private:
    typedef std::unordered_map<std::string, std::vector<std::shared_ptr<xd::sound>>> Audio_Cache;
    std::shared_ptr<xd::sound> load_sound(Audio_Cache& cache, const std::string& key,
        const std::string& filename, unsigned int channel_count = 1, bool pausable = true);
    std::shared_ptr<xd::music> load_music(Audio_Cache& cache, const std::string& filename);
    void load_audio_from_map_prop(Map& current_map, const std::string& prop_name, bool is_music);
    std::shared_ptr<xd::music> play_music_or_ambient(bool is_music, Map& current_map, std::optional<float> volume);
    std::shared_ptr<xd::music> play_music_or_ambient(bool is_music, Map& current_map, const std::string& filename,
        bool looping, std::optional<float> volume);
    std::shared_ptr<xd::music> play_music_or_ambient(bool is_music, Map& current_map,
        const std::shared_ptr<xd::music>& new_music, bool looping);
    std::shared_ptr<xd::music> music;
    std::shared_ptr<xd::music> ambient;
    std::shared_ptr<xd::audio> audio;
    Audio_Cache global_cache;
    std::string audio_folder;
    // was music already paused when game got paused?
    bool music_was_paused;
    bool ambient_was_paused;
};

#endif