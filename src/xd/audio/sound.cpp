#include "../../../include/xd/audio/sound.hpp"
#include "../../../include/xd/audio/audio.hpp"
#include "../../../include/xd/audio/detail/audio_handle.hpp"
#include "../../../include/xd/audio/exceptions.hpp"
#include "../../../include/log.hpp"
#include <FMOD/fmod.hpp>
#include <memory>

namespace xd { namespace detail {
    bool invalid_result(FMOD_RESULT result) {
        // Stolen and stale channel handles are expected
        return result != FMOD_OK
            && result != FMOD_ERR_CHANNEL_STOLEN
            && result != FMOD_ERR_INVALID_HANDLE;
    }
    struct sound_handle
    {
        detail::audio_handle* audio_handle;
        bool is_music;
        FMOD::Sound* sound;
        FMOD::Channel* channel;
        std::string filename;
        float volume;
        float pitch;
        sound_handle(detail::audio_handle* audio_handle, bool is_music) :
            audio_handle(audio_handle),
            is_music(is_music),
            sound(nullptr),
            channel(nullptr),
            volume(1.0f),
            pitch(1.0f) {}
        int get_loop_tag(const char* name) {
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
                    data = std::string{static_cast<const char*>(tag.data), tag.datalen};
                    value = std::stoi(data);
                    break;
                case FMOD_TAGDATATYPE_STRING_UTF8:
                    // Skip byte order mark
                    if (tag.datalen > 3 && ((unsigned char*)tag.data)[0] == 0xEF
                            && ((unsigned char*)tag.data)[1] == 0xBB
                            && ((unsigned char*)tag.data)[2] == 0xBF) {
                        offset = 3;
                    }
                    data = std::string{static_cast<const char*>(tag.data) + offset,
                        tag.datalen - offset};
                    value = std::stoi(data);
                    break;
                default:
                    LOGGER_W << "Unexpected audio tag type " << tag.datatype << " for " << name;
                    value = -1;
                }
            } catch (const std::invalid_argument& ex) {
                LOGGER_W << "Error in converting value of tag " << name
                    << " to int - tag value was: " << data << " - " << ex.what();
                value = -1;
            }
            return value;
        }
        ~sound_handle() {
            if (sound)
                sound->release();
        }
    };
    const FMOD_TIMEUNIT time_unit = FMOD_TIMEUNIT_PCM;
} }

xd::sound::sound(audio& audio, const std::string& filename, unsigned int flags)
{
    auto audio_handle = audio.get_handle();
    // load sound from file
    flags = flags ? flags : FMOD_LOOP_OFF | FMOD_2D;
    auto is_music = (flags & FMOD_CREATESTREAM) != 0;
    m_handle = std::make_unique<detail::sound_handle>(audio_handle, is_music);
    auto result = audio_handle->system->createSound(filename.c_str(),
        flags, nullptr,  &m_handle->sound);
    if (result != FMOD_OK) {
        throw audio_file_load_failed(filename, static_cast<int>(result));
    }
    m_handle->filename = filename;
    // Set loop points if specified in tags
    if (flags & FMOD_LOOP_NORMAL) {
        int loop_start = m_handle->get_loop_tag("LOOPSTART");
        int loop_end = m_handle->get_loop_tag("LOOPLENGTH");
        unsigned int u_length = 0;
        result = m_handle->sound->getLength(&u_length, detail::time_unit);
        if (result != FMOD_OK) {
            LOGGER_W << "Unable to get length of sound " << m_handle->filename << " - FMOD result: " << result;
        }
        int length = static_cast<int>(u_length);
        // Make sure loop start and end are within acceptable bounds
        if (loop_start < 0 || loop_start >= length)
            loop_start = 0;

        if (loop_end < 0)
            loop_end = m_handle->get_loop_tag("LOOPEND");
        else
            loop_end = loop_start + loop_end;

        if (loop_end < 0 || loop_end >= length)
            loop_end = length - 1;

        // Set loop points if needed
        if (loop_start != 0 || loop_end != length - 1) {
            LOGGER_I << "Setting loop points for " << filename << " to " << loop_start << ", " << loop_end;
            set_loop_points(loop_start, loop_end);
        }
    }
}

xd::sound::~sound()
{
}

void xd::sound::create_channel() {
    auto audio_handle = m_handle->audio_handle;
    auto result = audio_handle->system->playSound(m_handle->sound, nullptr, true, &m_handle->channel);
    if (result != FMOD_OK) {
        LOGGER_W << "Playback of sound file " << m_handle->filename << " failed - FMOD result: " << result;
    }
    result = m_handle->channel->setChannelGroup(m_handle->is_music ?
        audio_handle->music_channel_group : audio_handle->sound_channel_group);
    if (result != FMOD_OK) {
        LOGGER_W << "Unable to set channel group for " << m_handle->filename << " - FMOD result: " << result;
    }
    set_volume(m_handle->volume);
    set_pitch(m_handle->pitch);
}

void xd::sound::play()
{
    if (!paused()) {
        if (playing()) return;
        create_channel();
    }
    auto result = m_handle->channel->setPaused(false);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to play sound " << m_handle->filename << " - FMOD result: " << result;
    }
}

void xd::sound::pause()
{
    auto result = m_handle->channel->setPaused(true);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to pause sound " << m_handle->filename << " - FMOD result: " << result;
    }
}

void xd::sound::stop()
{
    auto result = m_handle->channel->stop();
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to stop sound " << m_handle->filename << " - FMOD result: " << result;
    }
}


bool xd::sound::playing() const
{
    if (!m_handle->channel) return false;
    bool playing = false;
    auto result = m_handle->channel->isPlaying(&playing);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to check if sound " << m_handle->filename << " is playing - FMOD result: " << result;
    }
    return playing;
}

bool xd::sound::paused() const
{
    if (!m_handle->channel) return false;
    bool paused = false;
    auto result = m_handle->channel->getPaused(&paused);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to check if sound " << m_handle->filename << " is paused - FMOD result: " << result;
    }
    return paused;
}

bool xd::sound::stopped() const
{
    return !playing();
}

void xd::sound::set_offset(unsigned int offset)
{
    auto result = m_handle->channel->setPosition(offset, detail::time_unit);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to set offset of sound " << m_handle->filename << " to " << offset << " - FMOD result: " << result;
    }
}

void xd::sound::set_volume(float volume)
{
    m_handle->volume = volume;
    auto result = m_handle->channel->setVolume(volume);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to set volume of sound " << m_handle->filename << " to " << volume << " - FMOD result: " << result;
    }
}

void xd::sound::set_pitch(float pitch)
{
    m_handle->pitch = pitch;
    auto result = m_handle->channel->setPitch(pitch);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to set pitch of sound " << m_handle->filename << " to " << pitch << " - FMOD result: " << result;
    }
}

void xd::sound::set_looping(bool looping)
{
    auto result = m_handle->sound->setLoopCount(looping ? -1 : 0);
    if (result != FMOD_OK) {
        LOGGER_W << "Unable to set looping of sound " << m_handle->filename << " to " << looping << " - FMOD result: " << result;
    }
}

void xd::sound::set_loop_points(unsigned int start, unsigned int end)
{
    if (end == 0u) {
        auto result = m_handle->sound->getLength(&end, detail::time_unit);
        if (result != FMOD_OK) {
            LOGGER_W << "Unable to get length of sound " << m_handle->filename << " - FMOD result: " << result;
        }
        end--;
    }
    auto result = m_handle->sound->setLoopPoints(start, detail::time_unit, end, detail::time_unit);
    if (result != FMOD_OK) {
        LOGGER_W << "Unable to set loop points of sound " << m_handle->filename << " to " << start << ", " << end << " - FMOD result: " << result;
    }
}

unsigned int xd::sound::get_offset() const
{
    unsigned int pos = 0;
    auto result = m_handle->channel->getPosition(&pos, detail::time_unit);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to get offset of sound " << m_handle->filename << " - FMOD result: " << result;
    }
    return pos;
}

float xd::sound::get_volume() const
{
    float volume = m_handle->volume;
    auto result = m_handle->channel->getVolume(&volume);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to get volume of sound " << m_handle->filename << " - FMOD result: " << result;
    }
    return volume;
}

float xd::sound::get_pitch() const
{
    float pitch = m_handle->pitch;
    auto result = m_handle->channel->getPitch(&pitch);
    if (detail::invalid_result(result)) {
        LOGGER_W << "Unable to get pitch of sound " << m_handle->filename << " - FMOD result: " << result;
    }
    return pitch;
}

bool xd::sound::get_looping() const
{
    int loop = 0;
    auto result = m_handle->sound->getLoopCount(&loop);
    if (result != FMOD_OK) {
        LOGGER_W << "Unable to get looping of sound " << m_handle->filename << " - FMOD result: " << result;
    }
    return loop != 0;
}

std::pair<unsigned int, unsigned int> xd::sound::get_loop_points() const
{
    auto start = 0u, end = 0u;
    auto result = m_handle->sound->getLoopPoints(&start, detail::time_unit, &end, detail::time_unit);
    if (result != FMOD_OK) {
        LOGGER_W << "Unable to get loop points of sound " << m_handle->filename << " - FMOD result: " << result;
    }
    return std::make_pair(start, end);
}

std::string xd::sound::get_filename() const {
    return m_handle->filename;
}