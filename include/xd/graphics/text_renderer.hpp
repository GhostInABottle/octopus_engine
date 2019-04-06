#ifndef H_XD_TEXT_RENDERER
#define H_XD_TEXT_RENDERER

#include "../config.hpp"
#include "../glm.hpp"
#include "font.hpp"
#include "text_formatter.hpp"
#include "shaders.hpp"
#include <string>

namespace xd
{
    class XD_API text_renderer
    {
    public:
        text_renderer();
        explicit text_renderer(shader_program::ptr shader);
        virtual ~text_renderer();

        void render(xd::font::ptr font, const xd::font_style& style, const mat4& mvp, const std::string& text);
        void render_formatted(xd::font::ptr font, xd::text_formatter::ptr formatter,
            const xd::font_style& style, const mat4& mvp, const std::string& text);

    private:
        shader_program::ptr m_shader;
    };
}

#endif
