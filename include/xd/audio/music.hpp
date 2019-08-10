#ifndef H_XD_AUDIO_MUSIC
#define H_XD_AUDIO_MUSIC

#include <string>
#include <utility>
#include <memory>

namespace xd
{
    namespace detail
    {
        struct music_handle;
    }
    class audio;

    class music
    {
    public:

        music(const music&) = delete;
        music& operator=(const music&) = delete;
        music(audio& audio, const std::string& filename, unsigned int flags = 0);
        virtual ~music();

        void play();
        void pause();
        void stop();

        bool playing() const;
        bool paused() const;
        bool stopped() const;

        void set_offset(unsigned int offset);
        void set_volume(float volume);
        void set_pitch(float pitch);
        void set_looping(bool looping);
        void set_loop_points(unsigned int start, unsigned int end = 0);

        unsigned int get_offset() const;
        float get_volume() const;
        float get_pitch() const;
        bool get_looping() const;
        std::pair<unsigned int, unsigned int> get_loop_points() const;

        std::string get_filename() const;

    private:
        std::unique_ptr<detail::music_handle> m_handle;
    };
}

#endif
