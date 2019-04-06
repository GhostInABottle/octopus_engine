#ifndef H_XD_AUDIO_AUDIO
#define H_XD_AUDIO_AUDIO

#include "../config.hpp"
#include "../ref_counted.hpp"
#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>

#pragma warning(disable: 4275)

namespace xd
{
    namespace detail
    {
        struct audio_handle;
    }

    class XD_API audio : public xd::ref_counted, public boost::noncopyable
    {
    public:
        typedef boost::intrusive_ptr<audio> ptr;

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
