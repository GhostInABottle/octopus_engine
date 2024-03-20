#include "../../../include/xd/audio/audio.hpp"
#include "../../../include/xd/audio/detail/fmod_audio_handle.hpp"

xd::audio::audio() : m_audio_handle(std::make_unique<detail::fmod_audio_handle>()) {}

xd::audio::~audio() {}

void xd::audio::set_music_volume(float volume) const {
    m_audio_handle->set_channel_group_volume(channel_group_type::music, volume);
}

float xd::audio::get_music_volume() const {
    return m_audio_handle->get_channel_group_volume(channel_group_type::music);
}

void xd::audio::set_sound_volume(float volume) const {
    m_audio_handle->set_channel_group_volume(channel_group_type::sound, volume);
    m_audio_handle->set_channel_group_volume(channel_group_type::non_pausable_sound, volume);
}

float xd::audio::get_sound_volume() const {
    return m_audio_handle->get_channel_group_volume(channel_group_type::sound);
}

void xd::audio::update() const {
    m_audio_handle->update();
}

void xd::audio::pause_sounds() {
    m_audio_handle->pause_channel_group(channel_group_type::sound);
}

void xd::audio::resume_sounds() {
    m_audio_handle->resume_channel_group(channel_group_type::sound);
}

xd::detail::audio_handle* xd::audio::get_handle() {
    return m_audio_handle.get();
}
