#ifndef H_XD_GRAPHICS_EXCEPTIONS
#define H_XD_GRAPHICS_EXCEPTIONS

#include "../exception.hpp"
#include <string>

namespace xd
{
    struct audio_system_init_failed : exception
    {
        audio_system_init_failed(const std::string& msg)
            : exception("failed to initialize audio system: " + msg)
        {
        }
    };
    struct audio_file_load_failed : exception
    {
        audio_file_load_failed(const std::string& filename)
            : exception("failed to load audio: " + filename)
        {
        }
    };
}

#endif

