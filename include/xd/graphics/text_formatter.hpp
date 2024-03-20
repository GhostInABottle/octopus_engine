#ifndef H_XD_GRAPHICS_TEXT_FORMATTER
#define H_XD_GRAPHICS_TEXT_FORMATTER

#include "detail/text_formatter.hpp"
#include "../glm.hpp"
#include "../vendor/utf8.h"
#include "exceptions.hpp"
#include "shader_program.hpp"
#include "sprite_batch.hpp"
#include <boost/lexical_cast.hpp>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace xd {
    class texture;
    class text_formatter;
    class font;

    class formatted_char {
    public:
        formatted_char(utf8::uint32_t chr)
            : m_chr(chr)
            , m_level(0) {}

        utf8::uint32_t get() const noexcept {
            return m_chr;
        }

    private:
        utf8::uint32_t m_chr;
        detail::text_formatter::state_change_list m_state_changes;
        int m_level;

        friend class text_decorator;
        friend class text_formatter;
        friend class detail::text_formatter::token_decorator;
        friend class detail::text_formatter::formatted_element_renderer;
    };

    class formatted_text {
    public:
        typedef std::vector<formatted_char>::iterator iterator;
        typedef std::vector<formatted_char>::const_iterator const_iterator;

        formatted_text() noexcept {}

        formatted_text(const std::string& text) {
            *this += text;
        }

        formatted_text& operator+=(const formatted_char& chr) {
            m_chars.push_back(chr);
            return *this;
        }

        formatted_text& operator+=(const formatted_text& text) {
            if (this != &text) {
                m_chars.insert(m_chars.end(), text.m_chars.begin(), text.m_chars.end());
            }
            return *this;
        }

        formatted_text& operator+=(const std::string& text) {
            std::string::const_iterator i = text.begin();
            while (i != text.end()) {
                auto c = utf8::next(i, text.end());
                m_chars.push_back(c);
            }
            return *this;
        }

        formatted_char& operator[](size_t index) noexcept {
            return m_chars[index];
        }

        const formatted_char& operator[](size_t index) const noexcept {
            return m_chars[index];
        }

        iterator begin() noexcept {
            return m_chars.begin();
        }

        iterator end() noexcept {
            return m_chars.end();
        }

        const_iterator begin() const noexcept {
            return m_chars.begin();
        }

        const_iterator end() const noexcept {
            return m_chars.end();
        }

        size_t length() const noexcept {
            return m_chars.size();
        }

        void clear() noexcept {
            m_chars.clear();
        }

        std::string get_unformatted() const {
            std::string unformatted;
            for (const_iterator i = m_chars.begin(); i != m_chars.end(); ++i) {
                utf8::append(i->get(), std::back_inserter(unformatted));
            }
            return unformatted;
        }

    private:
        std::vector<formatted_char> m_chars;

        friend class text_decorator;
        friend class text_formatter;
        friend class detail::text_formatter::token_decorator;
    };

    typedef std::variant<int, formatted_text> formatted_element;

    class text_decorator_args
    {
    public:
        template <typename T>
        T get(size_t n) const
        {
            if (n < m_args.size())
                return boost::lexical_cast<T>(m_args[n]);
            else
                throw text_formatter_exception("argument out of bounds");
        }

        template <typename T>
        T get(size_t n, const T& def) const
        {
            if (n < m_args.size())
                return boost::lexical_cast<T>(m_args[n]);
            else
                return def;
        }

        size_t count() const noexcept
        {
            return m_args.size();
        }

    private:
        std::vector<std::string> m_args;

        friend class text_formatter;
    };

    class text_decorator
    {
    public:
        text_decorator(int level);
        virtual ~text_decorator();

        void push_text(const formatted_text& text);
        void push_text(const std::string& text);
        void push_text(const formatted_char& chr);
        void push_text(utf8::uint32_t chr);

        void push_color(const glm::vec4& color);
        void push_alpha(float alpha);
        void push_size(int size);
        void push_type(const std::string& type);
        void push_shadow(const font_shadow& shadow);
        void push_outline(const font_outline& outline);
        void push_position(const glm::vec2& position);
        void push_letter_spacing(float letter_spacing);
        void push_force_autohint(bool force_autohint);

        void pop_color();
        void pop_alpha();
        void pop_size();
        void pop_type();
        void pop_shadow();
        void pop_outline();
        void pop_position();
        void pop_letter_spacing();
        void pop_force_autohint();

        template <typename T>
        void push_text(const T& val)
        {
            std::string str = boost::lexical_cast<std::string>(val);
            push_text(str);
        }

    private:
        // current state
        int m_current_level;
        formatted_text m_current_text;
        detail::text_formatter::state_change_list m_current_state_changes;

        // decorate_text needs to be able to modify current state
        friend class detail::text_formatter::token_decorator;
    };

    class text_formatter
    {
    public:

        typedef std::function<void (text_decorator&, const formatted_text&, const text_decorator_args&)> decorator_callback_t;
        typedef std::function<std::string (const std::string&)> variable_callback_t;

        text_formatter(std::shared_ptr<xd::texture> icon_texture = nullptr, vec2 icon_size = vec2{}, vec2 icon_offset = vec2{});
        virtual ~text_formatter();

        const std::string& get_decorator_open_delim() const;
        const std::string& get_decorator_open_escape_delim() const;
        const std::string& get_decorator_close_delim() const;
        const std::string& get_decorator_close_escape_delim() const;
        const std::string& get_decorator_terminate_delim() const;
        void set_decorator_open_delim(const std::string& delim);
        void set_decorator_open_escape_delim(const std::string& delim);
        void set_decorator_close_delim(const std::string& delim);
        void set_decorator_close_escape_delim(const std::string& delim);
        void set_decorator_terminate_delim(const std::string& delim);
        void set_decorator_delims(const std::string& open, const std::string& close, const std::string& terminate);
        const std::string& get_variable_open_delim() const;
        const std::string& get_variable_open_escape_delim() const;
        const std::string& get_variable_close_delim() const;
        const std::string& get_variable_close_escape_delim() const;
        void set_variable_open_delim(const std::string& delim);
        void set_variable_open_escape_delim(const std::string& delim);
        void set_variable_close_delim(const std::string& delim);
        void set_variable_close_escape_delim(const std::string& delim);
        void set_variable_delims(const std::string& open, const std::string& close);

        void register_decorator(const std::string& name, decorator_callback_t decorator);
        void register_variable(const std::string& name, variable_callback_t variable);

        void unregister_decorator(const std::string& name);
        void unregister_variable(const std::string& name);

        glm::vec2 render(const std::string& text, xd::font& font, const font_style& style,
            shader_program& shader, const glm::mat4& mvp, bool actual_rendering = true);

    private:
        typedef std::unordered_map<std::string, decorator_callback_t> decorator_list_t;
        typedef std::unordered_map<std::string, variable_callback_t> variable_list_t;

        // Icon rendering
        vec2 m_icon_size;
        vec2 m_icon_offset;
        std::shared_ptr<texture> m_icon_texture;
        sprite_batch m_icon_batch;

        // parse
        void parse(const std::string& text, std::list<detail::text_formatter::token>& tokens);

        // callbacks
        decorator_list_t m_decorators;
        variable_list_t m_variables;

        // delimiters
        std::string m_decorator_open_delim;
        std::string m_decorator_open_escape_delim;
        std::string m_decorator_close_delim;
        std::string m_decorator_close_escape_delim;
        std::string m_decorator_terminate_delim;
        std::string m_variable_open_delim;
        std::string m_variable_open_escape_delim;
        std::string m_variable_close_delim;
        std::string m_variable_close_escape_delim;
    };
}

#endif
