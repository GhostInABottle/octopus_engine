#include "../../../include/xd/audio/music.hpp"
#include "../../../include/xd/audio/exceptions.hpp"
#include "../../../include/xd/audio/sound.hpp"
#include <FMOD/fmod.hpp>
#include <memory>

namespace xd { namespace detail {
    struct music_handle
    {
        xd::sound sound;
        music_handle(const std::string& filename, unsigned int flags)
            : sound(filename, flags) {}
    };
} }

xd::music::music(const std::string& filename, unsigned int flags)
{
    // load music from file
    flags = flags ? flags : FMOD_CREATESTREAM | FMOD_LOOP_NORMAL | FMOD_2D;
    auto handle = std::unique_ptr<detail::music_handle>(new detail::music_handle(filename, flags));
    // all ok, release the memory to the real handle
    m_handle = handle.release();
}

xd::music::~music()
{
    delete m_handle;
}

void xd::music::play()
{
    m_handle->sound.play();
}

void xd::music::pause()
{
    m_handle->sound.pause();
}

void xd::music::stop()
{
    m_handle->sound.stop();
}

bool xd::music::playing() const
{
    return m_handle->sound.playing();
}

bool xd::music::paused() const
{
    return m_handle->sound.paused();
}

bool xd::music::stopped() const
{
    return m_handle->sound.stopped();
}

void xd::music::set_offset(unsigned int offset)
{
    m_handle->sound.set_offset(offset);
}

void xd::music::set_volume(float volume)
{
    m_handle->sound.set_volume(volume);
}

void xd::music::set_pitch(float pitch)
{
    m_handle->sound.set_pitch(pitch);
}

void xd::music::set_looping(bool looping)
{
    m_handle->sound.set_looping(looping);
}

void xd::music::set_loop_points(unsigned int start, unsigned int end)
{
    m_handle->sound.set_loop_points(start, end);
}

unsigned int xd::music::get_offset() const
{
    return m_handle->sound.get_offset();
}

float xd::music::get_volume() const
{
    return m_handle->sound.get_volume();
}

float xd::music::get_pitch() const
{
    return m_handle->sound.get_pitch();
}

bool xd::music::get_looping() const
{
    return m_handle->sound.get_looping();
}

std::pair<unsigned int, unsigned int> xd::music::get_loop_points() const
{
    return m_handle->sound.get_loop_points();
}

std::string xd::music::get_filename() const {
    return m_handle->sound.get_filename();
}
