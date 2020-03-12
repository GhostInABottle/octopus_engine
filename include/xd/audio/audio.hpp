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
        void set_global_volume(float volume) const;
        float get_global_volume() const;
        detail::audio_handle* get_handle() {
            return m_audio_handle.get();
        }

    private:
        std::unique_ptr<detail::audio_handle> m_audio_handle;
    };
}

#endif
