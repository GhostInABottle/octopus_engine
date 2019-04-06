#ifndef H_XD_GRAPHICS_FRAMEBUFFER
#define H_XD_GRAPHICS_FRAMEBUFFER

#include "../vendor/glew/glew.h"
#include "../ref_counted.hpp"
#include "../asset_serializer.hpp"
#include "texture.hpp"
#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <tuple>
#include <string>

namespace xd
{
    class XD_API framebuffer : public xd::ref_counted, public boost::noncopyable
    {
    public:
        typedef boost::intrusive_ptr<framebuffer> ptr;

        framebuffer();
        virtual ~framebuffer();

        void bind() const;
        void unbind() const;

        void attach_color_texture(xd::texture::ptr texture, int slot) const;
        void attach_depth_buffer(unsigned int id) const;

        static bool extension_supported() {
            return GLEW_EXT_framebuffer_object;
        }

        GLuint framebuffer_id() const {
            return m_buffer_id;
        }

        std::tuple<bool, std::string> check_complete() const;

    private:
        GLuint m_buffer_id;
    };

}

#endif
