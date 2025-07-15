#include "audio.hpp"
#include "music.hpp"
#include "sound.hpp"
#include <istream>
#include <memory>

xd::music::music(audio& audio, const std::string& filename,
        std::unique_ptr<std::istream> stream)
    : xd::sound(audio, filename, std::move(stream), channel_group_type::music) {}
