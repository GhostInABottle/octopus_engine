#ifndef H_XD_AUDIO_CHANNEL_GROUP_TYPE
#define H_XD_AUDIO_CHANNEL_GROUP_TYPE

enum class channel_group_type { music, sound, non_pausable_sound };

inline const char* channel_group_type_to_string(channel_group_type group_type) {
    if (group_type == channel_group_type::music) {
        return "music";
    } else if (group_type == channel_group_type::sound) {
        return "sound";
    } if (group_type == channel_group_type::non_pausable_sound) {
        return "non-pausable sound";
    }

    return "unknown";
}

#endif
