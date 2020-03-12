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
    result = m_audio_handle->system->createChannelGroup("music", &m_audio_handle->music_channel_group);
    if (result != FMOD_OK)
        throw audio_system_init_failed("Unable to create music channel group");
    result = m_audio_handle->system->createChannelGroup("sound", &m_audio_handle->sound_channel_group);
    if (result != FMOD_OK)
        throw audio_system_init_failed("Unable to create sound channel group");
}

xd::audio::~audio() {}

void xd::audio::set_music_volume(float volume) const {
    set_channel_group_volume(channel_group_type::music, volume);
}

float xd::audio::get_music_volume() const {
    return get_channel_group_volume(channel_group_type::music);
}

void xd::audio::set_sound_volume(float volume) const {
    set_channel_group_volume(channel_group_type::sound, volume);
}

float xd::audio::get_sound_volume() const {
    return get_channel_group_volume(channel_group_type::sound);
}

void xd::audio::set_channel_group_volume(channel_group_type group_type, float volume) const {
    auto group = group_type == channel_group_type::music
        ? m_audio_handle->music_channel_group
        : m_audio_handle->sound_channel_group;
    if (group) {
        group->setVolume(volume);
    }
}

float xd::audio::get_channel_group_volume(channel_group_type group_type) const {
    float volume = 0.0f;
    auto group = group_type == channel_group_type::music
        ? m_audio_handle->music_channel_group
        : m_audio_handle->sound_channel_group;
    if (group) {
        group->getVolume(&volume);
    }
    return volume;
}

void xd::audio::update() const {
    m_audio_handle->system->update();
}
