#ifndef H_XD_GRAPHICS_TEXTURE
#define H_XD_GRAPHICS_TEXTURE

#include "../vendor/glew/glew.h"
#include "../asset_serializer.hpp"
#include "image.hpp"
#include <boost/noncopyable.hpp>
#include <memory>
#include <string>

namespace xd
{
    class texture : public boost::noncopyable
    {
    public:

        texture(int width, int height, const void *data = 0, vec4 ck = vec4(0),
            GLint wrap_s = GL_REPEAT, GLint wrap_t = GL_REPEAT,
            GLint mag_filter = GL_LINEAR, GLint min_filter = GL_LINEAR);
        texture(const std::string& filename, vec4 ck = vec4(0),
            GLint wrap_s = GL_REPEAT, GLint wrap_t = GL_REPEAT,
            GLint mag_filter = GL_LINEAR, GLint min_filter = GL_LINEAR);
        texture(const xd::image& image,
            GLint wrap_s = GL_REPEAT, GLint wrap_t = GL_REPEAT,
            GLint mag_filter = GL_LINEAR, GLint min_filter = GL_LINEAR);
        virtual ~texture();

        void bind() const;
        void bind(int unit) const;

        void load(const std::string& filename, vec4 color_key = vec4(0));
        void load(const xd::image& image);
        void load(int width, int height, const void *data, vec4 color_key = vec4(0));
        void load(const void *data);
        void copy_read_buffer(int x, int y, int width, int height);

        GLuint texture_id() const;
        int width() const;
        int height() const;
        vec4 color_key() const;

        void set_wrap(GLint wrap_s, GLint wrap_t);
        void set_filter(GLint mag_filter, GLint min_filter);

    private:
        GLuint m_texture_id;
        int m_width;
        int m_height;
        vec4 m_color_key;

        void init();
    };

    template <>
    struct asset_serializer<xd::texture>
    {
        typedef std::string key_type;
        key_type operator()(const std::string& filename, xd::vec4,
            GLint = GL_REPEAT, GLint = GL_REPEAT,
            GLint = GL_LINEAR, GLint = GL_LINEAR) const
        {
            return filename;
        }
        key_type operator()(const xd::image& image,
            GLint = GL_REPEAT, GLint = GL_REPEAT,
            GLint = GL_LINEAR, GLint = GL_LINEAR) const
        {
            return image.filename();
        }
    };
}

#endif
