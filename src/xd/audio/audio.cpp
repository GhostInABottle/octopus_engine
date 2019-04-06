#include "../../../include/xd/audio/audio.hpp"
#include "../../../include/xd/audio/detail/audio_handle.hpp"
#include "../../../include/xd/audio/exceptions.hpp"
#include <memory>

xd::detail::audio_handle *xd::audio::m_audio_handle = nullptr;

void xd::audio::init()
{
    if (m_audio_handle) return;
    auto handle = std::unique_ptr<detail::audio_handle>(new detail::audio_handle);
    auto result = FMOD::System_Create(&handle->system);
    if (result != FMOD_OK)
        throw audio_system_init_failed("System creation failed");
    result = handle->system->init(512, FMOD_INIT_NORMAL, 0);
    if (result != FMOD_OK)
        throw audio_system_init_failed("System initialization failed");
    // all ok, release the memory to the real handle
    m_audio_handle = handle.release();
}

void xd::audio::update()
{
    m_audio_handle->system->update();
}

void xd::audio::shutdown()
{
    m_audio_handle->system->release();
    m_audio_handle = nullptr;
}
