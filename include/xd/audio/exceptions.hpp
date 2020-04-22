#ifndef H_XD_AUDIO_EXCEPTIONS
#define H_XD_AUDIO_EXCEPTIONS

#include "../exception.hpp"
#include <string>

namespace xd
{
    struct audio_system_init_failed : exception
    {
        audio_system_init_failed(const std::string& msg, int code)
            : exception("failed to initialize audio system: " + msg + ", error code: " + std::to_string(code))
        {
        }
    };
    struct audio_file_load_failed : exception
    {
        audio_file_load_failed(const std::string& filename, int code)
            : exception("failed to load audio: " + filename + ", error code: " + std::to_string(code))
        {
        }
    };
}

#endif

