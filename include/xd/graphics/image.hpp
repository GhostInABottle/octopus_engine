#ifndef H_XD_GRAPHICS_IMAGE
#define H_XD_GRAPHICS_IMAGE

#include "detail/image.hpp"

#include "../glm.hpp"
#include "../config.hpp"
#include "../ref_counted.hpp"
#include <boost/noncopyable.hpp>
#include <boost/intrusive_ptr.hpp>
#include <string>
#include <memory>

#ifndef XD_STATIC
// disable warning about boost::noncopyable not being dll-exportable
// as well as the private members that can't be accessed by client
#pragma warning(disable: 4275 4251)
#endif

namespace xd
{
    class XD_API image : public xd::ref_counted, public boost::noncopyable
    {
    public:
        typedef boost::intrusive_ptr<image> pt;

        image(const std::string& filename);
        image(const std::string& filename, xd::vec4 color_key);
        virtual ~image();

        void load(const std::string& filename);

        int width() const { return m_width; }
        int height() const { return m_height; }

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
