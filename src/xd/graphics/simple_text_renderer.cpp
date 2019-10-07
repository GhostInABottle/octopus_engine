#include "../../../include/xd/graphics/simple_text_renderer.hpp"
#include <memory>

xd::simple_text_renderer::simple_text_renderer(float width, float height)
    : m_shader(std::make_unique<xd::text_shader>())
{
    reset_projection(width, height);
}

xd::simple_text_renderer::simple_text_renderer(float width, float height, xd::shader_program* shader)
    : m_shader(shader)
{
    reset_projection(width, height);
}

xd::simple_text_renderer::~simple_text_renderer()
{
}

// setup an orthographic projection where 0 = top and height = bottom
void xd::simple_text_renderer::reset_projection(float width, float height) {
    m_projection = ortho(0.0f, width, 0.0f, height, -100.0f, 100.0f);
}

void xd::simple_text_renderer::render(xd::font& font, const xd::font_style& style, float x, float y, const std::string& text)
{
    glm::vec2 pos(x, y);
    font.render(text, style, m_shader.get(), m_projection, &pos);
}

void xd::simple_text_renderer::render_formatted(xd::font& font, xd::text_formatter& formatter,
    const xd::font_style& style, float x, float y, const std::string& text)
{
    auto mvp = xd::translate(m_projection, vec3(x, y, 0));
    formatter.render(text, font, style, *m_shader, mvp);
}
