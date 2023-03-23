#include "../../../include/xd/audio/music.hpp"
#include "../../../include/xd/audio/exceptions.hpp"
#include "../../../include/xd/audio/sound.hpp"
#include "../../../include/xd/audio/audio.hpp"
#include <memory>
#include <istream>

xd::music::music(audio& audio, const std::string& filename,
        std::unique_ptr<std::istream> stream)
    : xd::sound(audio, filename, std::move(stream), channel_group_type::music) {}
