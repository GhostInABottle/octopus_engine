#ifndef H_XD_AUDIO_AUDIO
#define H_XD_AUDIO_AUDIO

#include <boost/noncopyable.hpp>

namespace xd
{
    namespace detail
    {
        struct audio_handle;
    }

    class audio : public boost::noncopyable
    {
    public:
        static void init();
        static void update();
        static void shutdown();
        static detail::audio_handle *get_handle() {
            return m_audio_handle;
        }

    private:
        static detail::audio_handle *m_audio_handle;
    };
}

#endif
