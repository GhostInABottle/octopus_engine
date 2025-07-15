#include "fmod_sound_handle.hpp"
#include "fmod_audio_handle.hpp"
#include "../exceptions.hpp"
#include "../../../log.hpp"
#include <FMOD/fmod.hpp>
#include <istream>

namespace xd::detail {
    static bool invalid_result(FMOD_RESULT result) {
        // Stolen and stale channel handles are expected
        return result != FMOD_OK
            && result != FMOD_ERR_CHANNEL_STOLEN
            && result != FMOD_ERR_INVALID_HANDLE;
    }

    const FMOD_TIMEUNIT time_unit = FMOD_TIMEUNIT_PCM;
}

xd::detail::fmod_sound_handle::fmod_sound_handle(audio_handle& audio_handle,
        FMOD::Sound* sound, channel_group_type group_type, const std::string& filename,
        std::unique_ptr<std::istream> stream) :
    audio(audio_handle),
    stream(std::move(stream)),
    sound(sound),
    channel(nullptr),
    channel_group(group_type),
    filename(filename),
    volume(1.0f),
    pitch(1.0f) {}

int xd::detail::fmod_sound_handle::get_loop_tag(const char* name) {
    if (!sound)
        return -1;
    FMOD_TAG tag;
    int value = -1;
    if (sound->getTag(name, 0, &tag) != FMOD_OK) return value;
    std::string data;
    unsigned char offset = 0;
    try {
        switch (tag.datatype) {
        case FMOD_TAGDATATYPE_INT:
            value = *static_cast<int*>(tag.data);
            break;
        case FMOD_TAGDATATYPE_FLOAT:
            value = static_cast<int>(*static_cast<float*>(tag.data));
            break;
        case FMOD_TAGDATATYPE_STRING:
            data = std::string{ static_cast<const char*>(tag.data), tag.datalen };
            value = std::stoi(data);
            break;
        case FMOD_TAGDATATYPE_STRING_UTF8:
            // Skip byte order mark
            if (tag.datalen > 3 && ((unsigned char*)tag.data)[0] == 0xEF
                && ((unsigned char*)tag.data)[1] == 0xBB
                && ((unsigned char*)tag.data)[2] == 0xBF) {
                offset = 3;
            }
            data = std::string{ static_cast<const char*>(tag.data) + offset,
                tag.datalen - offset };
            value = std::stoi(data);
            break;
        default:
            LOGGER_W << "Unexpected audio tag type " << tag.datatype << " for " << name;
            value = -1;
        }
    }
    catch (const std::invalid_argument& ex) {
        LOGGER_W << "Error in converting value of tag " << name
            << " to int - tag value was: " << data << " - " << ex.what();
        value = -1;
    }
    return value;
}

xd::detail::tag_loop_info xd::detail::fmod_sound_handle::read_tagged_loop_points() {
    int loop_start = get_loop_tag("LOOPSTART");
    int loop_end = get_loop_tag("LOOPLENGTH");
    unsigned int u_length = 0;

    auto result = sound->getLength(&u_length, detail::time_unit);
    if (result != FMOD_OK) {
        LOGGER_W << "Unable to get length of sound " << filename << " - FMOD result: " << result;
    }

    int length = static_cast<int>(u_length);

    // Make sure loop start and end are within acceptable bounds
    if (loop_start < 0 || loop_start >= length)
        loop_start = 0;

    if (loop_end < 0)
        loop_end = get_loop_tag("LOOPEND");
    else
        loop_end = loop_start + loop_end;

    if (loop_end < 0 || loop_end >= length)
        loop_end = length - 1;

    return { loop_start, loop_end, length };
}

void xd::detail::fmod_sound_handle::create_channel() {
    audio.play_sound(*this);
    set_volume(volume);
    set_pitch(pitch);
}

void xd::detail::fmod_sound_handle::play() {
    if (!is_paused()) {
        if (is_playing()) return;
        create_channel();
    }
    auto result = channel->setPaused(false);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to play sound " << filename << " - FMOD result: " << result;
    }
}

bool xd::detail::fmod_sound_handle::is_playing() const {
    if (!channel) return false;

    bool playing = false;
    auto result = channel->isPlaying(&playing);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to check if sound " << filename << " is playing - FMOD result: " << result;
    }
    return playing;
}

void xd::detail::fmod_sound_handle::pause() {
    auto result = channel->setPaused(true);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to pause sound " << filename << " - FMOD result: " << result;
    }
}

bool xd::detail::fmod_sound_handle::is_paused() const {
    if (!channel) return false;
    bool paused = false;
    auto result = channel->getPaused(&paused);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to check if sound " << filename << " is paused - FMOD result: " << result;
    }
    return paused;
}

void xd::detail::fmod_sound_handle::stop() {
    auto result = channel->stop();
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to stop sound " << filename << " - FMOD result: " << result;
    }
}

void xd::detail::fmod_sound_handle::set_offset(unsigned int offset) {
    auto result = channel->setPosition(offset, detail::time_unit);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to set offset of sound " << filename << " to " << offset << " - FMOD result: " << result;
    }
}

unsigned int xd::detail::fmod_sound_handle::get_offset() const {
    unsigned int pos = 0;
    auto result = channel->getPosition(&pos, detail::time_unit);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to get offset of sound " << filename << " - FMOD result: " << result;
    }
    return pos;
}

void xd::detail::fmod_sound_handle::set_volume(float volume) {
    this->volume = volume;
    auto result = channel->setVolume(volume);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to set volume of sound " << filename << " to " << volume << " - FMOD result: " << result;
    }
}

float xd::detail::fmod_sound_handle::get_volume() const {
    float volume = this->volume;
    auto result = channel->getVolume(&volume);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to get volume of sound " << filename << " - FMOD result: " << result;
    }
    return volume;
}

void xd::detail::fmod_sound_handle::set_pitch(float pitch) {
    this->pitch = pitch;
    auto result = channel->setPitch(pitch);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to set pitch of sound " << filename << " to " << pitch << " - FMOD result: " << result;
    }
}

float xd::detail::fmod_sound_handle::get_pitch() const {
    float pitch = this->pitch;
    auto result = channel->getPitch(&pitch);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to get pitch of sound " << filename << " - FMOD result: " << result;
    }
    return pitch;
}

void xd::detail::fmod_sound_handle::set_looping(bool looping) {
    auto result = sound->setLoopCount(looping ? -1 : 0);
    if (result != FMOD_OK) {
        LOGGER_W << "Unable to set looping of sound " << filename << " to " << looping << " - FMOD result: " << result;
    }
}

bool xd::detail::fmod_sound_handle::is_looping() const {
    int loop = 0;
    auto result = sound->getLoopCount(&loop);
    if (result != FMOD_OK) {
        LOGGER_W << "Unable to get looping of sound " << filename << " - FMOD result: " << result;
    }
    return loop != 0;
}

void xd::detail::fmod_sound_handle::set_loop_points(unsigned int start, unsigned int end) {
    if (end == 0u) {
        auto result = sound->getLength(&end, detail::time_unit);
        if (result != FMOD_OK) {
            LOGGER_W << "Unable to get length of sound " << filename << " - FMOD result: " << result;
        }
        end--;
    }
    auto result = sound->setLoopPoints(start, detail::time_unit, end, detail::time_unit);
    if (result != FMOD_OK) {
        LOGGER_W << "Unable to set loop points of sound " << filename << " to " << start << ", " << end << " - FMOD result: " << result;
    }
}

std::pair<unsigned int, unsigned int> xd::detail::fmod_sound_handle::get_loop_points() const
{
    auto start = 0u, end = 0u;
    auto result = sound->getLoopPoints(&start, detail::time_unit, &end, detail::time_unit);
    if (result != FMOD_OK) {
        LOGGER_W << "Unable to get loop points of sound " << filename << " - FMOD result: " << result;
    }
    return std::make_pair(start, end);
}

xd::detail::fmod_sound_handle::~fmod_sound_handle() {
    if (sound) sound->release();
}
