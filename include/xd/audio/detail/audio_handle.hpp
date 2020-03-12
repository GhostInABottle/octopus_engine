#ifndef H_XD_AUDIO_DETAIL_AUDIO_HANDLE
#define H_XD_AUDIO_DETAIL_AUDIO_HANDLE

#include <FMOD/fmod.hpp>

namespace xd { namespace detail {
    struct audio_handle
    {
        FMOD::System* system{nullptr};
        FMOD::ChannelGroup* music_channel_group{nullptr};
        FMOD::ChannelGroup* sound_channel_group{nullptr};
        ~audio_handle() {
            if (music_channel_group)
                music_channel_group->release();
            if (sound_channel_group)
                sound_channel_group->release();
            if (system)
                system->release();
        }
    };
} }

#endif
