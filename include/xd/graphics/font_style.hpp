#ifndef H_XD_GRAPHICS_FONT_STYLE
#define H_XD_GRAPHICS_FONT_STYLE

#include "../glm.hpp"
#include "exceptions.hpp"
#include <optional>

namespace xd
{
    struct font_shadow
    {
        font_shadow() noexcept
            : x(1), y(1), color(vec4(0,0,0,1)) {}
        font_shadow(float x, float y, const vec4& color)
            : x(x), y(y), color(color) {}

        float x;
        float y;
        vec4 color;
    };

    struct font_outline
    {
        font_outline() noexcept
            : width(1), color(vec4(0,0,0,1)) {}
        font_outline(int width, const vec4& color)
            : width(width), color(color) {}

        int width;
        vec4 color;
    };

    class font_style
    {
    public:
        font_style(const vec4& color, int size)
            : m_color(color)
            , m_size(size)
            , m_letter_spacing(0)
            , m_line_height(0)
            , m_force_autohint(false)
        {
        }

        // setters for required styles
        font_style& color(const vec4& color) noexcept { m_color = color; return *this; }
        font_style& size(int size) noexcept { m_size = size; return *this; }
        font_style& letter_spacing(float letter_spacing) noexcept { m_letter_spacing = letter_spacing; return *this; }
        font_style& line_height(float line_height) noexcept { m_line_height = line_height; return *this; }
        font_style& force_autohint(bool force_autohint) noexcept { m_force_autohint = force_autohint; return *this; }

        // getters for required styles
        vec4 color() const noexcept { return m_color; }
        int size() const noexcept { return m_size; }
        float letter_spacing() const noexcept { return m_letter_spacing; }
        float line_height() const noexcept { return m_line_height; }
        bool force_autohint() const noexcept { return m_force_autohint; }

        // modifiable getters for required styles
        vec4& color() noexcept { return m_color; }
        int& size() noexcept { return m_size; }
        float& letter_spacing() noexcept { return m_letter_spacing; }
        float& line_height() noexcept { return m_line_height; }
        bool& force_autohint() noexcept { return m_force_autohint;  }

        // setters for optional styles
        font_style& type(const std::string& type) { m_type = type; return *this; }
        font_style& shadow(const font_shadow& shadow) { m_shadow = shadow; return *this; }
        font_style& shadow(float x, float y, const vec4& color) { m_shadow = font_shadow(x, y, color); return *this; }
        font_style& outline(int width, const vec4& color) { m_outline = font_outline(width, color); return *this; }
        font_style& outline(const font_outline& outline) { m_outline = outline; return *this; }

        // resetters for optional styles
        font_style& reset_type() noexcept { m_type = std::nullopt; return *this; }
        font_style& reset_shadow() noexcept { m_shadow = std::nullopt; return *this; }
        font_style& reset_outline() noexcept { m_outline = std::nullopt; return *this; }

        // checkers for optional styles
        bool has_type() const noexcept { return m_type.has_value(); }
        bool has_shadow() const noexcept { return m_shadow.has_value(); }
        bool has_outline() const noexcept { return m_outline.has_value(); }

        // getters for optional styles
        std::string type() const
        {
            if (!m_type)
                throw font_style_undefined("type");
            return *m_type;
        }

        font_shadow shadow() const
        {
            if (!m_shadow)
                throw font_style_undefined("shadow");
            return *m_shadow;
        }

        font_outline outline() const
        {
            if (!m_outline)
                throw font_style_undefined("outline");
            return *m_outline;
        }

        // modifiable getters for optional styles
        std::string& type()
        {
            if (!m_type)
                throw font_style_undefined("type");
            return *m_type;
        }

        font_shadow& shadow()
        {
            if (!m_shadow)
                throw font_style_undefined("shadow");
            return *m_shadow;
        }

        font_outline& outline()
        {
            if (!m_outline)
                throw font_style_undefined("outline");
            return *m_outline;
        }

    private:
        // font is a friend class
        friend class font;

        // required styles
        vec4 m_color;
        int m_size;
        float m_letter_spacing;
        float m_line_height;
        bool m_force_autohint;

        // optional styles
        std::optional<std::string> m_type;
        std::optional<font_shadow> m_shadow;
        std::optional<font_outline> m_outline;
    };
}

#endif
