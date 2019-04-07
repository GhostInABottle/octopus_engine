#ifndef H_XD_TEXT_RENDERER
#define H_XD_TEXT_RENDERER

#include "../glm.hpp"
#include "font.hpp"
#include "text_formatter.hpp"
#include "shaders.hpp"
#include <string>
#include <memory>

namespace xd
{
    class text_renderer
    {
    public:
        text_renderer();
        explicit text_renderer(shader_program* shader);
        virtual ~text_renderer();

        void render(xd::font& font, const xd::font_style& style, const mat4& mvp, const std::string& text);
        void render_formatted(xd::font& font, xd::text_formatter& formatter,
            const xd::font_style& style, const mat4& mvp, const std::string& text);

    private:
        std::unique_ptr<shader_program> m_shader;
    };
}

#endif
