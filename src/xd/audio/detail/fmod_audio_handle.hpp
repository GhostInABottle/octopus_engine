#ifndef H_XD_AUDIO_DETAIL_FMOD_AUDIO_HANDLE
#define H_XD_AUDIO_DETAIL_FMOD_AUDIO_HANDLE

#include "audio_handle.hpp"

namespace FMOD {
    class System;
    class ChannelGroup;
    class Sound;
    class Channel;
}

namespace xd::detail {
    class fmod_audio_handle : public audio_handle
    {
    public:
        // Setup the FMOD system
        fmod_audio_handle();
        // Create a sound object (initially paused)
        std::unique_ptr<sound_handle> create_sound(const std::string& filename,
            std::unique_ptr<std::istream> stream, channel_group_type group_type) override;
        // Play a sound
        void play_sound(sound_handle& sound) override;
        // Set volume for a group
        void set_channel_group_volume(channel_group_type group_type, float volume) override;
        // Set volume for a channel group
        float get_channel_group_volume(channel_group_type group_type) const override;
        // Pause or rsume a channel group
        void pause_channel_group(channel_group_type group_type) override;
        void resume_channel_group(channel_group_type group_type) override;
        bool is_channel_group_paused(channel_group_type group_type) const override;
        // Update the FMOD system
        void update() override;
        // Release FMOD objects
        ~fmod_audio_handle() override;
    private:
        FMOD::System* system{ nullptr };
        FMOD::ChannelGroup* music_channel_group{ nullptr };
        FMOD::ChannelGroup* sound_channel_group{ nullptr };
        FMOD::ChannelGroup* non_pausable_sound_channel_group{ nullptr };
        // Get group by type
        FMOD::ChannelGroup* get_channel_group(channel_group_type group_type) const;
    };
}

#endif
