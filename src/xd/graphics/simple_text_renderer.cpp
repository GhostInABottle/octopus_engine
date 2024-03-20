#include "../../../include/xd/graphics/font.hpp"
#include "../../../include/xd/graphics/shaders.hpp"
#include "../../../include/xd/graphics/simple_text_renderer.hpp"
#include "../../../include/xd/graphics/text_formatter.hpp"

xd::simple_text_renderer::simple_text_renderer()
    : m_shader(std::make_unique<xd::text_shader>())
{
}

xd::simple_text_renderer::simple_text_renderer(xd::shader_program* shader)
    : m_shader(shader)
{
}

xd::simple_text_renderer::~simple_text_renderer()
{
}

void xd::simple_text_renderer::render(xd::font& font, const xd::font_style& style, const xd::mat4& projection, float x, float y, const std::string& text)
{
    glm::vec2 pos(x, y);
    font.render(text, style, m_shader.get(), projection, &pos);
}

void xd::simple_text_renderer::render_formatted(xd::font& font, xd::text_formatter& formatter,
    const xd::font_style& style, const xd::mat4& projection, float x, float y, const std::string& text)
{
    auto mvp = xd::translate(projection, xd::round(vec3(x, y, 0)));
    formatter.render(text, font, style, *m_shader, mvp);
}

float xd::simple_text_renderer::text_width(xd::font& font, xd::text_formatter& formatter,
    const xd::font_style& style, const std::string& text) {
    return formatter.render(text, font, style, *m_shader, xd::mat4(), false).x;
}
