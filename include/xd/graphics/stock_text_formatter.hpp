#ifndef H_XD_GRAPHICS_STOCK_TEXT_FORMATTER
#define H_XD_GRAPHICS_STOCK_TEXT_FORMATTER

#include "../glm.hpp"
#include <string>
#include <ctime>
#include <unordered_map>
#include "text_formatter.hpp"

namespace xd
{
    class stock_text_formatter : public text_formatter
    {
    public:

        stock_text_formatter(const std::string& icons_filename = "", vec4 transparent_color = vec4{}, vec2 icon_size = vec2{}, vec2 icon_offset = vec2{});
        virtual ~stock_text_formatter();

        bool has_color(const std::string& name);
        glm::vec4 get_color(const std::string& name);
        void set_color(const std::string& name, const glm::vec4& color);

    private:
        typedef std::unordered_map<std::string, glm::vec4> color_map_t;

        void size_decorator(text_decorator& decorator, const formatted_text& text, const text_decorator_args& args);
        void type_decorator(text_decorator& decorator, const formatted_text& text, const text_decorator_args& args);
        void bold_decorator(text_decorator& decorator, const formatted_text& text, const text_decorator_args& args);
        void italic_decorator(text_decorator& decorator, const formatted_text& text, const text_decorator_args& args);
        void color_decorator(text_decorator& decorator, const formatted_text& text, const text_decorator_args& args);
        void shadow_decorator(text_decorator& decorator, const formatted_text& text, const text_decorator_args& args);
        void outline_decorator(text_decorator& decorator, const formatted_text& text, const text_decorator_args& args);
        void spacing_decorator(text_decorator& decorator, const formatted_text& text, const text_decorator_args& args);
        void rainbow_decorator(text_decorator& decorator, const formatted_text& text, const text_decorator_args& args);

        color_map_t m_colors;
    };
}

#endif
