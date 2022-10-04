#include "../../../include/xd/audio/music.hpp"
#include "../../../include/xd/audio/exceptions.hpp"
#include "../../../include/xd/audio/sound.hpp"
#include "../../../include/xd/audio/audio.hpp"
#include <memory>

xd::music::music(audio& audio, const std::string& filename)
    : xd::sound(audio, filename, channel_group_type::music) {}
