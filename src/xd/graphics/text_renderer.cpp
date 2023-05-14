#include "../../../include/xd/graphics/text_renderer.hpp"
#include "../../../include/xd/graphics/text_formatter.hpp"
#include "../../../include/xd/graphics/font.hpp"
#include <memory>


xd::text_renderer::text_renderer()
    : m_shader(std::make_unique<xd::text_shader>())
{
}

xd::text_renderer::text_renderer(xd::shader_program* shader)
    : m_shader(shader)
{
}

xd::text_renderer::~text_renderer()
{
}

void xd::text_renderer::render(xd::font& font, const xd::font_style& style, const xd::mat4& mvp, const std::string& text)
{
    font.render(text, style, m_shader.get(), mvp);
}

void xd::text_renderer::render_formatted(xd::font& font, xd::text_formatter& formatter,
    const xd::font_style& style, const xd::mat4& mvp, const std::string& text)
{
    formatter.render(text, font, style, *m_shader, mvp);
}
