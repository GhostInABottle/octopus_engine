#ifndef H_XD_GRAPHICS_FRAMEBUFFER
#define H_XD_GRAPHICS_FRAMEBUFFER

#include "../vendor/glew/glew.h"
#include <tuple>
#include <string>

namespace xd
{
    class texture;

    class framebuffer
    {
    public:
        framebuffer(const framebuffer&) = delete;
        framebuffer& operator=(const framebuffer&) = delete;
        framebuffer();
        virtual ~framebuffer();

        void bind() const;
        void unbind() const;

        void attach_color_texture(const xd::texture& texture, int slot) const;
        void attach_depth_buffer(const xd::texture& texture) const;

        static bool extension_supported() noexcept {
            return GLEW_EXT_framebuffer_object;
        }

        GLuint framebuffer_id() const noexcept {
            return m_buffer_id;
        }

        std::tuple<bool, std::string> check_complete() const;

    private:
        GLuint m_buffer_id;
    };

}

#endif
