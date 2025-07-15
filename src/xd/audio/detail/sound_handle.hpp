#ifndef H_XD_AUDIO_DETAIL_SOUND_HANDLE
#define H_XD_AUDIO_DETAIL_SOUND_HANDLE

#include <string>
#include "../channel_group_type.hpp"

namespace xd::detail {
    class audio_handle;

    struct tag_loop_info {
        int loop_start;
        int loop_end;
        int length;
        bool has_loop_points() {
            return loop_start != 0 || loop_end != length - 1;
        }
    };

    class sound_handle
    {
    public:
        virtual int get_loop_tag(const char* name) = 0;
        virtual tag_loop_info read_tagged_loop_points() = 0;
        virtual const std::string& get_filename() const = 0;

        virtual void play() = 0;
        virtual bool is_playing() const = 0;
        virtual void pause() = 0;
        virtual bool is_paused() const = 0;
        virtual void stop() = 0;
        virtual bool is_stopped() const = 0;

        virtual void set_offset(unsigned int offset) = 0;
        virtual unsigned int get_offset() const = 0;
        virtual void set_volume(float volume) = 0;
        virtual float get_volume() const = 0;
        virtual void set_pitch(float pitch) = 0;
        virtual float get_pitch() const = 0;

        virtual void set_looping(bool looping) = 0;
        virtual bool is_looping() const = 0;
        virtual void set_loop_points(unsigned int start, unsigned int end) = 0;
        virtual std::pair<unsigned int, unsigned int> get_loop_points() const = 0;
        virtual channel_group_type get_channel_group_type() const = 0;

        virtual ~sound_handle() noexcept {}
    };
}

#endif
