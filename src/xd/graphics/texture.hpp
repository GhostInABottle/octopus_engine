#ifndef H_XD_GRAPHICS_TEXTURE
#define H_XD_GRAPHICS_TEXTURE

#include "../vendor/glew/glew.h"
#include "image.hpp"
#include <iosfwd>
#include <memory>
#include <optional>
#include <string>

namespace xd
{
    class texture
    {
    public:
        texture(const texture&) = delete;
        texture& operator=(const texture&) = delete;
        texture();
        texture(int width, int height, const void *data = 0, vec4 ck = vec4(0),
            GLint wrap_s = GL_CLAMP_TO_EDGE, GLint wrap_t = GL_CLAMP_TO_EDGE,
            GLint mag_filter = GL_NEAREST, GLint min_filter = GL_NEAREST);
        texture(const std::string& filename, std::istream& stream, vec4 ck = vec4(0),
            GLint wrap_s = GL_CLAMP_TO_EDGE, GLint wrap_t = GL_CLAMP_TO_EDGE,
            GLint mag_filter = GL_NEAREST, GLint min_filter = GL_NEAREST);
        texture(const xd::image& image,
            GLint wrap_s = GL_CLAMP_TO_EDGE, GLint wrap_t = GL_CLAMP_TO_EDGE,
            GLint mag_filter = GL_NEAREST, GLint min_filter = GL_NEAREST);
        virtual ~texture();

        void bind() const noexcept;
        void bind(int unit) const noexcept;

        void load(const std::string& filename, std::istream& stream, vec4 color_key = vec4(0));
        void load(const xd::image& image);
        void load(int width, int height, const void *data, vec4 color_key = vec4(0));
        void load(const void *data) const;
        void copy_read_buffer(int x, int y, int width, int height);

        GLuint texture_id() const noexcept { return m_texture_id; }
        std::optional<std::string> filename() const noexcept { return m_filename; }
        int width() const noexcept { return m_width; }
        int height() const noexcept { return m_height; }
        vec4 color_key() const noexcept { return m_color_key; }

        void set_wrap(GLint wrap_s, GLint wrap_t) const;
        void set_filter(GLint mag_filter, GLint min_filter) const;

    private:
        GLuint m_texture_id;
        std::optional<std::string> m_filename;
        int m_width;
        int m_height;
        vec4 m_color_key;
        mutable int m_unit;

        void init();
    };
}

#endif
