#include "../../../include/xd/audio/sound.hpp"
#include "../../../include/xd/audio/audio.hpp"
#include "../../../include/xd/audio/detail/audio_handle.hpp"
#include "../../../include/xd/audio/detail/sound_handle.hpp"
#include "../../../include/xd/audio/exceptions.hpp"
#include "../../../include/log.hpp"
#include <memory>

xd::sound::sound(audio& audio, const std::string& filename,
        std::unique_ptr<std::istream> stream, channel_group_type group_type) {
    // load sound from file
    auto audio_handle = audio.get_handle();
    m_handle = audio_handle->create_sound(filename, std::move(stream), group_type);

    // Set loop points if specified in tags
    if (group_type != channel_group_type::music) return;

    auto tag_info = m_handle->read_tagged_loop_points();
    // Set loop points if needed
    if (!tag_info.has_loop_points()) return;

    LOGGER_I << "Setting loop points for " << filename << " to " <<
        tag_info.loop_start << ", " << tag_info.loop_end;
    set_loop_points(tag_info.loop_start, tag_info.loop_end);
}

xd::sound::~sound() {}

void xd::sound::play()
{
    m_handle->play();
}

void xd::sound::pause()
{
    m_handle->pause();
}

void xd::sound::stop()
{
    m_handle->stop();
}


bool xd::sound::playing() const
{
    return m_handle->is_playing();
}

bool xd::sound::paused() const
{
    return m_handle->is_paused();
}

bool xd::sound::stopped() const
{
    return m_handle->is_stopped();
}

void xd::sound::set_offset(unsigned int offset)
{
    m_handle->set_offset(offset);
}

void xd::sound::set_volume(float volume)
{
    m_handle->set_volume(volume);
}

void xd::sound::set_pitch(float pitch)
{
    m_handle->set_pitch(pitch);
}

void xd::sound::set_looping(bool looping)
{
    m_handle->set_looping(looping);
}

void xd::sound::set_loop_points(unsigned int start, unsigned int end)
{
    m_handle->set_loop_points(start, end);
}

unsigned int xd::sound::get_offset() const
{
    return m_handle->get_offset();
}

float xd::sound::get_volume() const
{
    return m_handle->get_volume();
}

float xd::sound::get_pitch() const
{
    return m_handle->get_pitch();
}

bool xd::sound::looping() const
{
    return m_handle->is_looping();
}

std::pair<unsigned int, unsigned int> xd::sound::get_loop_points() const
{
    return m_handle->get_loop_points();
}

channel_group_type xd::sound::get_channel_group_type() const {
    return m_handle->get_channel_group_type();
}

std::string xd::sound::get_filename() const {
    return m_handle->get_filename();
}