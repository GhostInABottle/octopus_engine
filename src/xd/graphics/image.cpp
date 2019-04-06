#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_FAILURE_USERMSG // readable error messages
#include "../../../include/xd/vendor/stb/stb_image.h"
#include "../../../include/xd/graphics/exceptions.hpp"
#include "../../../include/xd/graphics/image.hpp"

namespace xd { namespace detail { namespace image {

    struct handle
    {
        void* data = nullptr;
    };

} } }

xd::image::image(const std::string& filename)
    : image(filename, xd::vec4(0))
{
}

xd::image::image(const std::string& filename, xd::vec4 color_key)
    : m_width(0)
    , m_height(0)
    , m_color_key(color_key)
    , m_image(detail::image::handle_ptr(new detail::image::handle))
{
    load(filename);
}

xd::image::~image()
{
    stbi_image_free(m_image->data);
}

void xd::image::load(const std::string& filename)
{
    if (m_image->data)
        stbi_image_free(m_image->data);

    int channels;
    m_image->data = stbi_load(filename.c_str(), &m_width, &m_height, &channels, 4);

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
