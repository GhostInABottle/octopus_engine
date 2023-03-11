#ifndef H_XD_AUDIO_DETAIL_FMOD_SOUND_HANDLE
#define H_XD_AUDIO_DETAIL_FMOD_SOUND_HANDLE

#include "sound_handle.hpp"

namespace FMOD {
    class Sound;
    class Channel;
}

namespace xd::detail {
    class fmod_sound_handle : public sound_handle {
    public:
        fmod_sound_handle(audio_handle& audio_handle, FMOD::Sound* sound,
            channel_group_type group_type, const std::string& filename);

        int get_loop_tag(const char* name) override;
        tag_loop_info read_tagged_loop_points() override;

        void play() override;
        bool is_playing() const override;
        void pause() override;
        bool is_paused() const override;
        void stop() override;
        bool is_stopped() const override { return !is_playing(); }

        void set_offset(unsigned int offset) override;
        unsigned int get_offset() const override;
        void set_volume(float volume) override;
        float get_volume() const override;
        void set_pitch(float pitch) override;
        float get_pitch() const override;

        void set_looping(bool looping) override;
        bool is_looping() const override;
        void set_loop_points(unsigned int start, unsigned int end) override;
        std::pair<unsigned int, unsigned int> get_loop_points() const override;

        FMOD::Sound* get_sound() { return sound; }
        void set_channel(FMOD::Channel* new_channel) { channel = new_channel; }
        FMOD::Channel* get_channel() { return channel; }
        channel_group_type get_channel_group_type() const override { return channel_group; }
        const std::string& get_filename() const override { return filename; }

        ~fmod_sound_handle() override;
    private:
        audio_handle& audio;
        FMOD::Sound* sound;
        FMOD::Channel* channel;
        channel_group_type channel_group;
        std::string filename;
        float volume;
        float pitch;
        void create_channel();
    };
}

#endif
