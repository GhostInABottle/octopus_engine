#ifndef H_XD_TEXT_RENDERER
#define H_XD_TEXT_RENDERER

#include "../glm.hpp"
#include "shader_program.hpp"
#include <string>
#include <memory>

namespace xd
{
    class font;
    class font_style;
    class text_formatter;

    class text_renderer
    {
    public:
        text_renderer();
        explicit text_renderer(shader_program* shader);
        virtual ~text_renderer();

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
