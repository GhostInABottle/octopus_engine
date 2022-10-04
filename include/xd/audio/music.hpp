#ifndef H_XD_AUDIO_MUSIC
#define H_XD_AUDIO_MUSIC

#include "sound.hpp"

namespace xd
{
    class audio;
    class sound;

    class music : public sound
    {
    public:

        music(const music&) = delete;
        music& operator=(const music&) = delete;
        music(audio& audio, const std::string& filename);
    };
}

#endif
