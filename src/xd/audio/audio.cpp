#include "../../../include/xd/audio/audio.hpp"
#include "../../../include/xd/audio/detail/audio_handle.hpp"
#include "../../../include/xd/audio/exceptions.hpp"
#include <FMOD/fmod.hpp>
#include <memory>

xd::audio::audio() {
    m_audio_handle = std::make_unique<detail::audio_handle>();
    auto result = FMOD::System_Create(&m_audio_handle->system);
    if (result != FMOD_OK)
        throw audio_system_init_failed("System creation failed");
    result = m_audio_handle->system->init(512, FMOD_INIT_NORMAL, 0);
    if (result != FMOD_OK)
        throw audio_system_init_failed("System initialization failed");
}

xd::audio::~audio() {}

void xd::audio::set_global_volume(float volume) const {
    FMOD::ChannelGroup* group = nullptr;
    m_audio_handle->system->getMasterChannelGroup(&group);
    if (group) {
        group->setVolume(volume);
    }
}

float xd::audio::get_global_volume() const {
    float volume = 0.0f;
    FMOD::ChannelGroup* group = nullptr;
    m_audio_handle->system->getMasterChannelGroup(&group);
    if (group) {
        group->getVolume(&volume);
    }
    return volume;
}

void xd::audio::update() const {
    m_audio_handle->system->update();
}
