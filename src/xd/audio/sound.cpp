#include "../../../include/xd/audio/sound.hpp"
#include "../../../include/xd/audio/audio.hpp"
#include "../../../include/xd/audio/detail/audio_handle.hpp"
#include "../../../include/xd/audio/exceptions.hpp"
#include "../../../include/log.hpp"
#include <FMOD/fmod.hpp>
#include <memory>

namespace xd { namespace detail {
    struct sound_handle
    {
        detail::audio_handle* audio_handle;
        bool is_music;
        FMOD::Sound* sound;
        FMOD::Channel* channel;
        std::string filename;
        sound_handle(detail::audio_handle* audio_handle, bool is_music) :
            audio_handle(audio_handle), is_music(is_music), sound(nullptr), channel(nullptr) {}
        int get_loop_tag(const char* name) {
            if (!sound)
                return -1;
            FMOD_TAG tag;
            int value = -1;
            if (sound->getTag(name, 0, &tag) == FMOD_OK) {
                if (tag.datatype == FMOD_TAGDATATYPE_STRING) {
                    std::string data = static_cast<const char*>(tag.data);
                    value = std::stoi(data);
                } else {
                    value = -1;
                }
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
    if (result != FMOD_OK) throw audio_file_load_failed(filename);
    m_handle->filename = filename;

    // Set loop points if specified in tags
    if (flags & FMOD_LOOP_NORMAL) {
        int loop_start = m_handle->get_loop_tag("LOOPSTART");
        int loop_end = m_handle->get_loop_tag("LOOPLENGTH");
        unsigned int u_length = 0;
        m_handle->sound->getLength(&u_length, detail::time_unit);
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
        if (loop_start != 0 || loop_end != length - 1)
            set_loop_points(loop_start, loop_end);
    }
}

xd::sound::~sound()
{
}

void xd::sound::play()
{
    if (m_handle->channel && playing() && !paused()) return;

    if (!m_handle->channel || !paused()) {
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
    }
    m_handle->channel->setPaused(false);
}

void xd::sound::pause()
{
    m_handle->channel->setPaused(true);
}

void xd::sound::stop()
{
    m_handle->channel->stop();
}


bool xd::sound::playing() const
{
    bool playing = false;
    m_handle->channel->isPlaying(&playing);
    return playing;
}

bool xd::sound::paused() const
{
    bool paused = false;
    m_handle->channel->getPaused(&paused);
    return paused;
}

bool xd::sound::stopped() const
{
    return !playing();
}

void xd::sound::set_offset(unsigned int offset)
{
    m_handle->channel->setPosition(offset, detail::time_unit);
}

void xd::sound::set_volume(float volume)
{
    m_handle->channel->setVolume(volume);
}

void xd::sound::set_pitch(float pitch)
{
    m_handle->channel->setPitch(pitch);
}

void xd::sound::set_looping(bool looping)
{
    m_handle->sound->setLoopCount(looping ? -1 : 0);
}

void xd::sound::set_loop_points(unsigned int start, unsigned int end)
{
    if (end == 0) {
        m_handle->sound->getLength(&end, detail::time_unit);
        end--;
    }
    m_handle->sound->setLoopPoints(start, detail::time_unit, end, detail::time_unit);
}

unsigned int xd::sound::get_offset() const
{
    unsigned int pos = 0;
    m_handle->channel->getPosition(&pos, detail::time_unit);
    return pos;
}

float xd::sound::get_volume() const
{
    float volume = 0.0f;
    m_handle->channel->getVolume(&volume);
    return volume;
}

float xd::sound::get_pitch() const
{
    float pitch = 0.0f;
    m_handle->channel->getPitch(&pitch);
    return pitch;
}

bool xd::sound::get_looping() const
{
    int loop = 0;
    m_handle->sound->getLoopCount(&loop);
    return loop != 0;
}

std::pair<unsigned int, unsigned int> xd::sound::get_loop_points() const
{
    unsigned int start = 0, end = 0;
    m_handle->sound->getLoopPoints(&start, detail::time_unit, &end, detail::time_unit);
    return std::make_pair(start, end);
}

std::string xd::sound::get_filename() const {
    return m_handle->filename;
}