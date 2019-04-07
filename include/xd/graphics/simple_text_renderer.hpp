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
        simple_text_renderer(float width, float height);
        simple_text_renderer(float width, float height, shader_program* shader);
        virtual ~simple_text_renderer();

        void reset_projection(float width, float height);
        void render(xd::font& font, const xd::font_style& style, float x, float y, const std::string& text);
        void render_formatted(xd::font& font, xd::text_formatter& formatter,
            const xd::font_style& style, float x, float y, const std::string& text);

    private:
        mat4 m_projection;
        std::unique_ptr<shader_program> m_shader;
    };
}

#endif
