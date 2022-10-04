#include "../../../../include/xd/audio/detail/fmod_audio_handle.hpp"
#include "../../../../include/xd/audio/detail/fmod_sound_handle.hpp"
#include "../../../../include/xd/audio/exceptions.hpp"
#include "../../../../include/log.hpp"

#include <FMOD/fmod.hpp>

xd::detail::fmod_audio_handle::fmod_audio_handle() {
    auto result = FMOD::System_Create(&system);
    if (result != FMOD_OK)
        throw audio_system_init_failed("System creation failed", static_cast<int>(result));

    result = system->init(512, FMOD_INIT_NORMAL, 0);
    if (result != FMOD_OK)
        throw audio_system_init_failed("System initialization failed", static_cast<int>(result));

    result = system->createChannelGroup("music", &music_channel_group);
    if (result != FMOD_OK)
        throw audio_system_init_failed("Unable to create music channel group", static_cast<int>(result));

    result = system->createChannelGroup("sound", &sound_channel_group);
    if (result != FMOD_OK)
        throw audio_system_init_failed("Unable to create sound channel group", static_cast<int>(result));

    result = system->createChannelGroup("non_pausable_sound", &non_pausable_sound_channel_group);
    if (result != FMOD_OK)
        throw audio_system_init_failed("Unable to create non-pausable sound channel group", static_cast<int>(result));
}

std::unique_ptr<xd::detail::sound_handle> xd::detail::fmod_audio_handle::create_sound(const std::string& filename, channel_group_type group_type) {
    auto flags = group_type == channel_group_type::music
        ? FMOD_CREATESTREAM | FMOD_LOOP_NORMAL | FMOD_2D
        : FMOD_LOOP_OFF | FMOD_2D;

    FMOD::Sound* sound = nullptr;
    auto result = system->createSound(filename.c_str(),
        flags, nullptr, &sound);

    if (result != FMOD_OK) {
        throw audio_file_load_failed(filename, static_cast<int>(result));
    }

    return std::make_unique<xd::detail::fmod_sound_handle>(*this, sound, group_type, filename);
}

void xd::detail::fmod_audio_handle::play_sound(sound_handle& sound) {
    auto& fmod_sound = dynamic_cast<fmod_sound_handle&>(sound);
    FMOD::Channel* channel = nullptr;
    auto channel_group = get_channel_group(fmod_sound.get_channel_group_type());

    auto result = system->playSound(fmod_sound.get_sound(),
        channel_group, true, &channel);
    if (result != FMOD_OK) {
        LOGGER_W << "Playback of sound file " << fmod_sound.get_filename() << " failed - FMOD result: " << result;
    }

    fmod_sound.set_channel(channel);
}

// Get group by type
FMOD::ChannelGroup* xd::detail::fmod_audio_handle::get_channel_group(channel_group_type group_type) const {
    return group_type == channel_group_type::music
        ? music_channel_group
        : (group_type == channel_group_type::non_pausable_sound
            ? non_pausable_sound_channel_group
            : sound_channel_group);
}

void xd::detail::fmod_audio_handle::set_channel_group_volume(channel_group_type group_type, float volume) {
    auto group = get_channel_group(group_type);
    auto result = group->setVolume(volume);
    if (result != FMOD_OK) {
        auto type_name = channel_group_type_to_string(group_type);
        LOGGER_W << "Failed to set volume for channel group " << type_name << " - FMOD result : " << result;
    }
}

float xd::detail::fmod_audio_handle::get_channel_group_volume(channel_group_type group_type) const {
    float volume = 0.0f;
    auto group = get_channel_group(group_type);

    auto result = group->getVolume(&volume);
    if (result != FMOD_OK) {
        auto type_name = channel_group_type_to_string(group_type);
        LOGGER_W << "Failed to get volume for channel group " << type_name << " - FMOD result : " << result;
    }

    return volume;
}

void xd::detail::fmod_audio_handle::pause_channel_group(channel_group_type group_type) {
    auto result = get_channel_group(group_type)->setPaused(true);
    if (result != FMOD_OK) {
        auto type_name = channel_group_type_to_string(group_type);
        LOGGER_W << "Failed to pause channel group " << type_name << " - FMOD result : " << result;
    }
}

void xd::detail::fmod_audio_handle::resume_channel_group(channel_group_type group_type) {
    auto result = get_channel_group(group_type)->setPaused(false);
    if (result != FMOD_OK) {
        auto type_name = channel_group_type_to_string(group_type);
        LOGGER_W << "Failed to resume channel group " << type_name << " - FMOD result : " << result;
    }
}

bool xd::detail::fmod_audio_handle::is_channel_group_paused(channel_group_type group_type) const {
    auto paused = false;
    auto group = get_channel_group(group_type);

    auto result = group->getPaused(&paused);
    if (result != FMOD_OK) {
        auto type_name = channel_group_type_to_string(group_type);
        LOGGER_W << "Failed to check if channel group " << type_name << " is paused - FMOD result : " << result;
    }

    return paused;
}

void xd::detail::fmod_audio_handle::update() {
    system->update();
}

xd::detail::fmod_audio_handle::~fmod_audio_handle() {
    if (music_channel_group)
        music_channel_group->release();
    if (sound_channel_group)
        sound_channel_group->release();
    if (non_pausable_sound_channel_group)
        non_pausable_sound_channel_group->release();
    if (system)
        system->release();
}
