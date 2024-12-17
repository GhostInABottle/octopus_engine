#include "../../../include/xd/graphics/font.hpp"
#include "../../../include/xd/graphics/shaders.hpp"
#include "../../../include/xd/graphics/text_formatter.hpp"
#include "../../../include/xd/graphics/text_renderer.hpp"
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

void xd::text_renderer::render(xd::font& font, const xd::font_style& style, const xd::mat4& projection,
        float x, float y, const std::string& text) {
    font.render(text, style, m_shader.get(), projection, xd::vec2(x, y));
}

void xd::text_renderer::render_formatted(xd::font& font, xd::text_formatter& formatter,
    const xd::font_style& style, const xd::mat4& projection, float x, float y, const std::string& text) {
    auto mvp = xd::translate(projection, xd::round(vec3(x, y, 0)));
    formatter.render(text, font, style, *m_shader, mvp);
}

float xd::text_renderer::text_width(xd::font& font, xd::text_formatter& formatter,
    const xd::font_style& style, const std::string& text) {
    return formatter.render(text, font, style, *m_shader, xd::mat4(), false).x;
}
