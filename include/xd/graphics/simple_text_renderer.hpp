#ifndef H_XD_SIMPLE_TEXT_RENDERER
#define H_XD_SIMPLE_TEXT_RENDERER

#include "../glm.hpp"
#include "font.hpp"
#include "text_formatter.hpp"
#include "shaders.hpp"
#include <string>
#include <memory>

namespace xd
{
    class simple_text_renderer
    {
    public:
        simple_text_renderer();
        simple_text_renderer(shader_program* shader);
        virtual ~simple_text_renderer();

        void render(xd::font& font, const xd::font_style& style, const xd::mat4& projection, float x, float y, const std::string& text);
        void render_formatted(xd::font& font, xd::text_formatter& formatter,
            const xd::font_style& style, const xd::mat4& projection, float x, float y, const std::string& text);

        float text_width(xd::font& font, xd::text_formatter& formatter,
            const xd::font_style& style, const std::string& text);

    private:
        std::unique_ptr<shader_program> m_shader;
    };
}

#endif
