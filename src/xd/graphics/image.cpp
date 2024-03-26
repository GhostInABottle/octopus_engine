#include "../../../include/xd/graphics/exceptions.hpp"
#include "../../../include/xd/graphics/image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_FAILURE_USERMSG // readable error messages
#include "../../../include/xd/vendor/stb/stb_image.h"
#include <fstream>

namespace xd { namespace detail { namespace image {

    struct handle
    {
        void* data = nullptr;
    };

    static int stream_read(void* user, char* data, int size) {
        auto stream = static_cast<std::istream*>(user);
        stream->read(data, size);
        return static_cast<int>(stream->gcount());
    }

    static void stream_skip(void* user, int n) {
        auto stream = static_cast<std::istream*>(user);
        stream->seekg(n, std::ios_base::cur);
    }

    static int stream_eof(void* user) {
        auto stream = static_cast<std::istream*>(user);
        return stream->eof() ? 1 : 0;
    }

    static stbi_io_callbacks stream_callbacks{
       stream_read,
       stream_skip,
       stream_eof,
    };

} } }

xd::image::image(const std::string& filename, std::istream& stream)
    : image(filename, stream, xd::vec4(0))
{
}

xd::image::image(const std::string& filename, std::istream& stream, xd::vec4 color_key)
    : m_width(0)
    , m_height(0)
    , m_color_key(color_key)
    , m_image(std::make_shared<detail::image::handle>())
{
    load(filename, stream);
}

xd::image::~image()
{
    stbi_image_free(m_image->data);
}

void xd::image::load(const std::string& filename, std::istream& stream)
{
    if (m_image->data) {
        stbi_image_free(m_image->data);
    }

    int channels;
    m_image->data = stbi_load_from_callbacks(&detail::image::stream_callbacks,
        static_cast<void*>(&stream), &m_width, &m_height, &channels, 4);

    if (!m_image->data)
        throw failed_to_load_image(filename, stbi_failure_reason());

    m_filename = filename;
}

void *xd::image::data()
{
    return m_image->data;
}

const void *xd::image::data() const
{
    return m_image->data;
}
