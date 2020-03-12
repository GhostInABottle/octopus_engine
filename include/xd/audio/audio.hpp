#ifndef H_XD_AUDIO_AUDIO
#define H_XD_AUDIO_AUDIO

#include <memory>

namespace xd
{
    namespace detail
    {
        struct audio_handle;
    }

    class audio
    {
    public:
        audio(const audio&) = delete;
        audio& operator=(const audio&) = delete;
        audio();
        ~audio();
        void update() const;
        void set_music_volume(float volume) const;
        float get_music_volume() const;
        void set_sound_volume(float volume) const;
        float get_sound_volume() const;
        detail::audio_handle* get_handle() {
            return m_audio_handle.get();
        }
    private:
        enum class channel_group_type { music, sound };
        void set_channel_group_volume(channel_group_type group_type, float volume) const;
        float get_channel_group_volume(channel_group_type group_type) const;
        std::unique_ptr<detail::audio_handle> m_audio_handle;
    };
}

#endif
