#ifndef H_XD_AUDIO_DETAIL_AUDIO_HANDLE
#define H_XD_AUDIO_DETAIL_AUDIO_HANDLE

#include <memory>
#include <string>
#include <iosfwd>
#include "../channel_group_type.hpp"


namespace xd::detail {
    class sound_handle;

    class audio_handle
    {
    public:
        // Create a sound instance
        virtual std::unique_ptr<sound_handle> create_sound(const std::string& filename,
            std::unique_ptr<std::istream> stream, channel_group_type group_type) = 0;
        // Set volume for a group
        virtual void set_channel_group_volume(channel_group_type group_type, float volume) = 0;
        // Set volume for a channel group
        virtual float get_channel_group_volume(channel_group_type group_type) const = 0;
        // Pause or resume a channel group
        virtual void pause_channel_group(channel_group_type group_type) = 0;
        virtual void resume_channel_group(channel_group_type group_type) = 0;
        virtual bool is_channel_group_paused(channel_group_type group_type) const = 0;
        // Play a sound
        virtual void play_sound(sound_handle& sound) = 0;
        // Update the audio system
        virtual void update() = 0;
        // Release resources
        virtual ~audio_handle() {}
    };
}

#endif
