#ifndef H_XD_AUDIO_AUDIO
#define H_XD_AUDIO_AUDIO

#include <memory>

namespace xd
{
    namespace detail
    {
        class audio_handle;
        class sound_handle;
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
        void pause_sounds();
        void resume_sounds();
        detail::audio_handle* get_handle();
    private:
        std::unique_ptr<detail::audio_handle> m_audio_handle;
    };
}

#endif
