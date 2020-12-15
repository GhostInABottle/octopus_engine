#ifndef H_XD_GRAPHICS_IMAGE
#define H_XD_GRAPHICS_IMAGE

#include "detail/image.hpp"

#include "../glm.hpp"
#include <string>
#include <memory>


namespace xd
{
    class image
    {
    public:

        image(const std::string& filename);
        image(const std::string& filename, xd::vec4 color_key);
        virtual ~image();
        image(const image&) = delete;
        image& operator=(const image&) = delete;

        void load(const std::string& filename);

        int width() const noexcept { return m_width; }
        int height() const noexcept { return m_height; }

        std::string filename() const { return m_filename; }
        xd::vec4 color_key() const { return m_color_key; }

        void *data();
        const void *data() const;

    private:
        detail::image::handle_ptr m_image;
        int m_width;
        int m_height;
        std::string m_filename;
        xd::vec4 m_color_key;
    };
}

#endif
