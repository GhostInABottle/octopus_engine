#ifndef H_XD_AUDIO_AUDIO
#define H_XD_AUDIO_AUDIO

#include <boost/noncopyable.hpp>
#include <memory>

namespace xd
{
    namespace detail
    {
        struct audio_handle;
    }

    class audio : public boost::noncopyable
    {
    public:
        audio();
        ~audio();
        void update();
        detail::audio_handle* get_handle() {
            return m_audio_handle.get();
        }

    private:
        std::unique_ptr<detail::audio_handle> m_audio_handle;
    };
}

#endif
