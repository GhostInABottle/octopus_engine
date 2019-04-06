#ifndef H_XD_AUDIO_DETAIL_AUDIO_HANDLE
#define H_XD_AUDIO_DETAIL_AUDIO_HANDLE

#include <FMOD/fmod.hpp>

namespace xd { namespace detail {
    struct audio_handle
    {
        FMOD::System* system;
        audio_handle() : system(nullptr) {}
        ~audio_handle() {
            if (system)
                system->release();
        }
    };
} }

#endif
