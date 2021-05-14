#include "../../../include/xd/graphics/exceptions.hpp"
#include "../../../include/xd/graphics/text_formatter.hpp"
#include "../../../include/xd/graphics/texture.hpp"
#include "../../../include/xd/glm.hpp"
#include "../../../include/xd/vendor/utf8.h"
#include <string>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <utility>
#include <list>
#include <cctype>

namespace xd { namespace detail { namespace text_formatter {

    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    template< typename Iter >
    bool safe_equal(std::string str1, Iter begin2, Iter end2)
    {
        return end2 - begin2 >= str1.end() - str1.begin() && std::equal(str1.begin(), str1.end(), begin2);
    }

    // token types
    struct token_text
    {
        std::string text;
    };

    struct token_icon
    {
        int index;
    };

    struct token_variable
    {
        std::string name;
        xd::text_formatter::variable_callback_t callback;
    };

    struct token_open_decorator
    {
        std::string name;
        text_decorator_args args;
        xd::text_formatter::decorator_callback_t callback;
    };

    struct token_close_decorator
    {
        std::string name;
    };

    // expand variables
    class expand_variables
    {
    public:
        expand_variables(std::list<detail::text_formatter::token>& tokens)
            : m_tokens(tokens)
        {
        }

        void operator()(const token_text& tok)
        {
            // if we have tokens
            if (m_tokens.size() != 0) {
                // if previous token is text
                token_text *prev_tok = std::get_if<token_text>(&m_tokens.back());
                if (prev_tok) {
                    // append variable to the previous token
                    prev_tok->text += tok.text;
                    return;
                }
            }

            // previous token wasn't text, push the value as new token
            m_tokens.push_back(tok);
        }

        void operator()(const token_variable& tok)
        {
            // evaluate the variable
            std::string value = tok.callback(tok.name);

            // if we have tokens
            if (m_tokens.size() != 0) {
                // if previous token is text
                token_text *prev_tok = std::get_if<token_text>(&m_tokens.back());
                if (prev_tok) {
                    // append variable to the previous token
                    prev_tok->text += value;
                    return;
                }
            }

            // previous token wasn't text, push the value as new token
            token_text new_tok;
            new_tok.text = value;
            m_tokens.push_back(new_tok);
        }

        void operator()(const token_icon& tok) {
            // don't do anything to decorators
            m_tokens.push_back(tok);
        }

        void operator()(const token_open_decorator& tok)
        {
            // don't do anything to decorators
            m_tokens.push_back(tok);
        }

        void operator()(const token_close_decorator& tok)
        {
            // don't do anything to decorators
            m_tokens.push_back(tok);
        }

    private:
        std::list<detail::text_formatter::token>& m_tokens;
    };

    // decorate text and icons
    class token_decorator
    {
    public:
        token_decorator(std::list<formatted_element>& elements)
            : m_elements(elements)
            , m_prev_close_decorator(false)
            , m_level(0)
        {
        }

        std::list<formatted_element>::iterator find_formatted_text() {
            return std::find_if(m_elements.begin(), m_elements.end(),
                [](formatted_element& e) { return std::holds_alternative<formatted_text>(e); });
        }

        void operator()(const token_text& tok)
        {
            // set level for each character
            formatted_text text{tok.text};
            for (auto& formatted_char : text) {
                formatted_char.m_level = m_level;
            }

            // text node, create a formatted version of it
            auto first_text = m_elements.empty() ? nullptr : std::get_if<formatted_text>(&m_elements.front());
            if (!m_prev_close_decorator && first_text) {
                first_text->m_chars.insert(first_text->m_chars.begin(),
                    text.begin(), text.end());
            } else {
                m_elements.push_front(text);
            }

            m_prev_close_decorator = false;
        }

        void operator()(const token_variable& tok)
        {
            // do nothing; variables are already expanded
        }

        void operator()(const token_icon& tok)
        {
            m_elements.push_front(tok.index);
            m_prev_close_decorator = false;
        }

        void operator()(const token_open_decorator& tok)
        {
            // if there are no associated text node, create an empty one
            if (m_prev_close_decorator) {
                m_elements.push_front(formatted_text{});
            }

            auto first_text_iterator = find_formatted_text();
            if (first_text_iterator == m_elements.end()) {
                m_elements.push_back(formatted_text{});
                first_text_iterator = std::prev(m_elements.end());
            }

            auto text = std::get_if<formatted_text>(&*first_text_iterator);
            // decorate the associated text node
            xd::text_decorator decorator(m_level);
            tok.callback(decorator, *text, tok.args);
            if (decorator.m_current_text.length()) {
                // set level for each character
                formatted_text::iterator i = decorator.m_current_text.begin();
                while (i != decorator.m_current_text.end()) {
                    if (m_level > i->m_level)
                        i->m_level = m_level;
                    ++i;
                }
            }

            auto next_iterator = std::next(first_text_iterator);
            auto next_element = next_iterator != m_elements.end() ? std::get_if<formatted_text>(&*next_iterator) : nullptr;
            if (next_element && next_element->begin()->m_level == (m_level - 1)) {
                // if we're inside another decorator, concatenate with previous string
                next_element->m_chars.insert(next_element->m_chars.begin(),
                    decorator.m_current_text.begin(), decorator.m_current_text.end());
                m_elements.erase(first_text_iterator);
            } else {
                text->m_chars.assign(decorator.m_current_text.begin(), decorator.m_current_text.end());
            }

            // tokens are iterated in reverse order, hence we decrement
            m_prev_close_decorator = false;
            m_level--;
        }

        void operator()(const token_close_decorator& tok)
        {
            // tokens are iterated in reverse order, hence we increment
            m_prev_close_decorator = true;
            m_level++;
        }

    private:
        std::list<formatted_element>& m_elements;
        bool m_prev_close_decorator;
        int m_level;
    };

    struct stacked_font_style
    {
        stacked_font_style(const font_style& initial_state)
        {
            colors.push_back(nested_color{initial_state.color(), 0});
            sizes.push_back(nested_size{initial_state.size(), 0});
            letter_spacings.push_back(nested_letter_spacing{initial_state.letter_spacing(), 0});
            force_autohints.push_back(nested_force_autohint{initial_state.force_autohint(), 0});

            if (initial_state.has_type()) {
                types.push_back(nested_type{initial_state.type(), 0});
            }

            if (initial_state.has_shadow()) {
                shadows.push_back(nested_shadow{initial_state.shadow(), 0});
            }

            if (initial_state.has_outline()) {
                outlines.push_back(nested_outline{initial_state.outline(), 0});
            }

        }

        font_style get_font_style()
        {
            font_style style(colors.back().value, sizes.back().value);
            style.letter_spacing(letter_spacings.back().value);
            style.force_autohint(force_autohints.back().value);
            if (alphas.size() != 0)
                style.color().a *= alphas.back().value;
            if (types.size() != 0)
                style.type(types.back().value);
            if (shadows.size() != 0)
                style.shadow(shadows.back().value);
            if (outlines.size() != 0)
                style.outline(outlines.back().value);
            return style;
        }

        template <typename T, typename V>
        void push(std::list<T>& list, const V& value, int level)
        {
            int current_level = (list.size() != 0 ? list.back().level : -1);
            if (level >= current_level) {
                T state_change;
                state_change.value = value;
                state_change.level = level;
                list.push_back(state_change);
            }
        }

        void push_color(const glm::vec4& value, int level)
        {
            if (colors.back().value.a != 1.0f) {
                glm::vec4 color = value;
                color.a *= colors.back().value.a;
                push(colors, color, level);
            } else {
                push(colors, value, level);
            }
        }

        void push_alpha(float value, int level)
        {
            if (alphas.size() != 0)
                push(alphas, value * alphas.back().value, level);
            else
                push(alphas, value, level);
        }

        void push_size(int value, int level)
        {
            push(sizes, value, level);
        }

        void push_type(const std::string& value, int level)
        {
            push(types, value, level);
        }

        void push_shadow(const font_shadow& value, int level)
        {
            push(shadows, value, level);
        }

        void push_outline(const font_outline& value, int level)
        {
            push(outlines, value, level);
        }

        void push_position(const glm::vec2& value, int level)
        {
            if (positions.size() != 0)
                push(positions, value + positions.back().value, level);
            else
                push(positions, value, level);
        }

        void push_letter_spacing(float value, int level)
        {
            push(letter_spacings, value, level);
        }

        void push_force_autohint(bool value, int level)
        {
            push(force_autohints, value, level);
        }

        template <typename T>
        void pop(std::list<T>& list, int level)
        {
            if (list.size() == 0 || list.back().level != level)
                throw xd::text_formatter_exception("no value to pop");
            list.pop_back();
        }

        void pop_color(int level)
        {
            pop(colors, level);
        }

        void pop_alpha(int level)
        {
            pop(alphas, level);
        }

        void pop_size(int level)
        {
            pop(sizes, level);
        }

        void pop_type(int level)
        {
            pop(types, level);
        }

        void pop_shadow(int level)
        {
            pop(shadows, level);
        }

        void pop_outline(int level)
        {
            pop(outlines, level);
        }

        void pop_position(int level)
        {
            pop(positions, level);
        }

        void pop_letter_spacing(int level)
        {
            pop(letter_spacings, level);
        }

        void pop_force_autohint(int level)
        {
            pop(force_autohints, level);
        }

        template <typename T>
        void pop_level(std::list<T>& list, int level)
        {
            typename std::list<T>::iterator i = list.begin();
            while (i != list.end()) {
                if (i->level > level)
                    i = list.erase(i);
                else
                    ++i;
            }
        }

        void pop_level(int level)
        {
            pop_level(colors, level);
            pop_level(sizes, level);
            pop_level(alphas, level);
            pop_level(types, level);
            pop_level(shadows, level);
            pop_level(outlines, level);
            pop_level(positions, level);
            pop_level(letter_spacings, level);
            pop_level(force_autohints, level);
        }

        std::list<nested_color> colors;
        std::list<nested_size> sizes;
        std::list<nested_alpha> alphas;
        std::list<nested_type> types;
        std::list<nested_shadow> shadows;
        std::list<nested_outline> outlines;
        std::list<nested_position> positions;
        std::list<nested_letter_spacing> letter_spacings;
        std::list<nested_force_autohint> force_autohints;
    };


    class apply_state_changes
    {
    public:
        apply_state_changes(stacked_font_style& style_stack)
            : m_style_stack(style_stack)
        {
        }

        void operator()(const state_change_push_color& state_change)
        {
            m_style_stack.push_color(state_change.value, state_change.level);
        }

        void operator()(const state_change_push_alpha& state_change)
        {
            m_style_stack.push_alpha(state_change.value, state_change.level);
        }

        void operator()(const state_change_push_size& state_change)
        {
            m_style_stack.push_size(state_change.value, state_change.level);
        }

        void operator()(const state_change_push_type& state_change)
        {
            m_style_stack.push_type(state_change.value, state_change.level);
        }

        void operator()(const state_change_push_shadow& state_change)
        {
            m_style_stack.push_shadow(state_change.value, state_change.level);
        }

        void operator()(const state_change_push_outline& state_change)
        {
            m_style_stack.push_outline(state_change.value, state_change.level);
        }

        void operator()(const state_change_push_position& state_change)
        {
            m_style_stack.push_position(state_change.value, state_change.level);
        }

        void operator()(const state_change_push_letter_spacing& state_change)
        {
            m_style_stack.push_letter_spacing(state_change.value, state_change.level);
        }

        void operator()(const state_change_push_force_autohint& state_change)
        {
            m_style_stack.push_force_autohint(state_change.value, state_change.level);
        }

        void operator()(const state_change_pop_color& state_change)
        {
            m_style_stack.pop_color(state_change.level);
        }

        void operator()(const state_change_pop_alpha& state_change)
        {
            m_style_stack.pop_alpha(state_change.level);
        }

        void operator()(const state_change_pop_size& state_change)
        {
            m_style_stack.pop_size(state_change.level);
        }

        void operator()(const state_change_pop_type& state_change)
        {
            m_style_stack.pop_type(state_change.level);
        }

        void operator()(const state_change_pop_shadow& state_change)
        {
            m_style_stack.pop_shadow(state_change.level);
        }

        void operator()(const state_change_pop_outline& state_change)
        {
            m_style_stack.pop_outline(state_change.level);
        }

        void operator()(const state_change_pop_position& state_change)
        {
            m_style_stack.pop_position(state_change.level);
        }

        void operator()(const state_change_pop_letter_spacing& state_change)
        {
            m_style_stack.pop_letter_spacing(state_change.level);
        }

        void operator()(const state_change_pop_force_autohint& state_change)
        {
            m_style_stack.pop_force_autohint(state_change.level);
        }
    private:
        stacked_font_style& m_style_stack;
    };

} } }

xd::text_decorator::text_decorator(int level)
    : m_current_level(level)
{
}

xd::text_decorator::~text_decorator()
{
}

void xd::text_decorator::push_text(const xd::formatted_text& text)
{
    if (text.length() != 0) {
        if (m_current_state_changes.size() != 0) {
            formatted_text format_text = text;
            detail::text_formatter::state_change_list&
                state_changes = format_text.begin()->m_state_changes;
            state_changes.insert(state_changes.begin(), m_current_state_changes.begin(), m_current_state_changes.end());
            m_current_text += format_text;
            m_current_state_changes.clear();
        } else {
            m_current_text += text;
        }
    }
}

void xd::text_decorator::push_text(const std::string& text)
{
    push_text(formatted_text{text});
}

void xd::text_decorator::push_text(const formatted_char& chr)
{
    formatted_text text;
    text += chr;
    push_text(text);
}

void xd::text_decorator::push_text(utf8::uint32_t chr)
{
    push_text(formatted_char(chr));
}

void xd::text_decorator::push_color(const glm::vec4& value)
{
    detail::text_formatter::state_change_push_color state_change{value, m_current_level};
    m_current_state_changes.push_back(state_change);
}

void xd::text_decorator::push_alpha(float value)
{
    detail::text_formatter::state_change_push_alpha state_change{value, m_current_level};
    m_current_state_changes.push_back(state_change);
}

void xd::text_decorator::push_size(int value)
{
    detail::text_formatter::state_change_push_size state_change{value, m_current_level};
    m_current_state_changes.push_back(state_change);
}

void xd::text_decorator::push_type(const std::string& value)
{
    detail::text_formatter::state_change_push_type state_change{value, m_current_level};
    m_current_state_changes.push_back(state_change);
}

void xd::text_decorator::push_shadow(const font_shadow& value)
{
    detail::text_formatter::state_change_push_shadow state_change{value, m_current_level};
    m_current_state_changes.push_back(state_change);
}

void xd::text_decorator::push_outline(const font_outline& value)
{
    detail::text_formatter::state_change_push_outline state_change{value, m_current_level};
    m_current_state_changes.push_back(state_change);
}

void xd::text_decorator::push_position(const glm::vec2& value)
{
    detail::text_formatter::state_change_push_position state_change{value, m_current_level};
    m_current_state_changes.push_back(state_change);
}

void xd::text_decorator::push_letter_spacing(float value)
{
    detail::text_formatter::state_change_push_letter_spacing state_change{value, m_current_level};
    m_current_state_changes.push_back(state_change);
}

void xd::text_decorator::push_force_autohint(bool value)
{
    detail::text_formatter::state_change_push_force_autohint state_change{value, m_current_level};
    m_current_state_changes.push_back(state_change);
}

void xd::text_decorator::pop_color()
{
    detail::text_formatter::state_change_pop_color state_change{m_current_level};
    m_current_state_changes.push_back(state_change);
}

void xd::text_decorator::pop_alpha()
{
    detail::text_formatter::state_change_pop_alpha state_change{m_current_level};
    m_current_state_changes.push_back(state_change);
}

void xd::text_decorator::pop_size()
{
    detail::text_formatter::state_change_pop_size state_change{m_current_level};
    m_current_state_changes.push_back(state_change);
}

void xd::text_decorator::pop_type()
{
    detail::text_formatter::state_change_pop_type state_change{m_current_level};
    m_current_state_changes.push_back(state_change);
}

void xd::text_decorator::pop_shadow()
{
    detail::text_formatter::state_change_pop_shadow state_change{m_current_level};
    m_current_state_changes.push_back(state_change);
}

void xd::text_decorator::pop_outline()
{
    detail::text_formatter::state_change_pop_outline state_change{m_current_level};
    m_current_state_changes.push_back(state_change);
}

void xd::text_decorator::pop_position()
{
    detail::text_formatter::state_change_pop_position state_change{m_current_level};
    m_current_state_changes.push_back(state_change);
}

void xd::text_decorator::pop_letter_spacing()
{
    detail::text_formatter::state_change_pop_letter_spacing state_change{m_current_level};
    m_current_state_changes.push_back(state_change);
}

void xd::text_decorator::pop_force_autohint()
{
    detail::text_formatter::state_change_pop_force_autohint state_change{m_current_level};
    m_current_state_changes.push_back(state_change);
}


xd::text_formatter::text_formatter(const std::string& icons_filename, vec4 transparent_color, vec2 icon_size)
        : m_decorator_open_delim("{")
        , m_decorator_open_escape_delim("{{")
        , m_decorator_close_delim("}")
        , m_decorator_close_escape_delim("}}")
        , m_decorator_terminate_delim("/")
        , m_variable_open_delim("$<")
        , m_variable_open_escape_delim("$$<<")
        , m_variable_close_delim(">")
        , m_variable_close_escape_delim(">>")
        , m_icon_size(icon_size) {
    if (icons_filename.empty()) return;
    m_icon_texture = std::make_shared<xd::texture>(icons_filename, transparent_color,
        GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
}

xd::text_formatter::~text_formatter()
{
}

const std::string& xd::text_formatter::get_decorator_open_delim() const
{
    return m_decorator_open_delim;
}

const std::string& xd::text_formatter::get_decorator_open_escape_delim() const {
    return m_decorator_open_escape_delim;
}

const std::string& xd::text_formatter::get_decorator_close_delim() const
{
    return m_decorator_close_delim;
}

const std::string& xd::text_formatter::get_decorator_close_escape_delim() const {
    return m_decorator_close_escape_delim;
}

const std::string& xd::text_formatter::get_decorator_terminate_delim() const
{
    return m_decorator_terminate_delim;
}

void xd::text_formatter::set_decorator_open_delim(const std::string& delim)
{
    m_decorator_open_delim = delim;
}

void xd::text_formatter::set_decorator_open_escape_delim(const std::string& delim) {
    m_decorator_open_escape_delim = delim;
}

void xd::text_formatter::set_decorator_close_delim(const std::string& delim)
{
    m_decorator_close_delim = delim;
}

void xd::text_formatter::set_decorator_close_escape_delim(const std::string& delim) {
    m_decorator_close_escape_delim = delim;
}

void xd::text_formatter::set_decorator_terminate_delim(const std::string& delim)
{
    m_decorator_terminate_delim = delim;
}

void xd::text_formatter::set_decorator_delims(const std::string& open, const std::string& close, const std::string& terminate)
{
    m_decorator_open_delim = open;
    m_decorator_close_delim = close;
    m_decorator_terminate_delim = terminate;
    m_decorator_open_escape_delim = open + open;
    m_decorator_close_escape_delim = close + close;
}

const std::string& xd::text_formatter::get_variable_open_delim() const
{
    return m_variable_open_delim;
}

const std::string& xd::text_formatter::get_variable_open_escape_delim() const {
    return m_variable_open_escape_delim;
}

const std::string& xd::text_formatter::get_variable_close_delim() const
{
    return m_variable_close_delim;
}

const std::string& xd::text_formatter::get_variable_close_escape_delim() const {
    return m_variable_close_escape_delim;
}

void xd::text_formatter::set_variable_open_delim(const std::string& delim)
{
    m_variable_open_delim = delim;
}

void xd::text_formatter::set_variable_open_escape_delim(const std::string& delim) {
    m_variable_open_escape_delim = delim;
}

void xd::text_formatter::set_variable_close_delim(const std::string& delim)
{
    m_variable_close_delim = delim;
}

void xd::text_formatter::set_variable_close_escape_delim(const std::string& delim) {
    m_variable_close_escape_delim = delim;
}

void xd::text_formatter::register_decorator(const std::string& name, xd::text_formatter::decorator_callback_t decorator)
{
    // make sure it's not already registered
    decorator_list_t::iterator i = m_decorators.find(name);
    if (i != m_decorators.end()) {
        throw text_formatter_exception("decorator \""+name+"\" already registered");
    }

    m_decorators[name] = decorator;
}

void xd::text_formatter::register_variable(const std::string& name, xd::text_formatter::variable_callback_t variable)
{
    // make sure it's not already registered
    variable_list_t::iterator i = m_variables.find(name);
    if (i != m_variables.end()) {
        throw text_formatter_exception("variable \""+name+"\" already registered");
    }

    m_variables[name] = variable;
}

void xd::text_formatter::unregister_decorator(const std::string& name)
{
    // make sure it's registered
    decorator_list_t::iterator i = m_decorators.find(name);
    if (i == m_decorators.end()) {
        throw text_formatter_exception("variable \""+name+"\" not registered");
    }

    m_decorators.erase(i);
}

void xd::text_formatter::unregister_variable(const std::string& name)
{
    // make sure it's registered
    variable_list_t::iterator i = m_variables.find(name);
    if (i == m_variables.end()) {
        throw text_formatter_exception("variable \""+name+"\" not registered");
    }

    m_variables.erase(i);
}

void xd::text_formatter::set_variable_delims(const std::string& open, const std::string& close)
{
    m_variable_open_delim = open;
    m_variable_close_delim = close;
    m_variable_open_escape_delim = open + open;
    m_variable_close_escape_delim = close + close;
}

glm::vec2 xd::text_formatter::render(const std::string& text, xd::font& font, const xd::font_style& style,
    xd::shader_program& shader, const glm::mat4& mvp, bool actual_rendering) {
    // parse the input
    std::list<detail::text_formatter::token> tokens;
    parse(text, tokens);

    // first pass: replace variables and concatenate strings
    std::list<detail::text_formatter::token> expanded_tokens;
    detail::text_formatter::expand_variables expand_variables_step(expanded_tokens);
    for (auto& token : tokens) {
        std::visit(expand_variables_step, token);
    }

    // second pass: iterate through the tokens in reverse order, decorating the text
    std::list<formatted_element> elements;
    detail::text_formatter::token_decorator decorate_text_step(elements);
    for (auto i = tokens.rbegin(); i != tokens.rend(); ++i) {
        std::visit(decorate_text_step, *i);
    }

    // render text
    detail::text_formatter::stacked_font_style style_stack(style);
    glm::vec2 pos;
    int current_level = 0;

    auto render_icon = [&](int icon_index) {
        if (!m_icon_texture) return;
        if (actual_rendering) {
            auto icons_per_row = static_cast<int>(m_icon_texture->width() / m_icon_size.x);
            rect src{
                static_cast<float>(icon_index % icons_per_row) * m_icon_size.x,
                static_cast<float>(icon_index / icons_per_row) * m_icon_size.y,
                m_icon_size.x,
                m_icon_size.y
            };
            auto style = style_stack.get_font_style();
            auto color = vec4{1.0f, 1.0f, 1.0f, style.color().a};
            m_icon_batch.add(m_icon_texture, src, pos.x, pos.y, color);
        }
        pos.x += m_icon_size.x;
    };

    auto render_text = [&](const formatted_text& text) {
        // iterate through each char until a style change is met
        std::string current_str;

        for (auto& formatted_char : text) {
            if (formatted_char.m_state_changes.size() || formatted_char.m_level < current_level) {
                // draw the current string using current style
                if (current_str.length() != 0) {
                    if (!style_stack.positions.empty())
                        pos += style_stack.positions.back().value;
                    font.render(current_str, style_stack.get_font_style(), &shader, mvp, &pos, actual_rendering);
                    if (!style_stack.positions.empty())
                        pos -= style_stack.positions.back().value;
                    current_str.clear();
                }

                // process the styles
                detail::text_formatter::apply_state_changes change_current_style(style_stack);
                for (auto& state_change : formatted_char.m_state_changes) {
                    std::visit(change_current_style, state_change);
                }
            }

            // purge styles from the previous level if the new level is smaller
            if (formatted_char.m_level < current_level) {
                style_stack.pop_level(formatted_char.m_level);
            }

            // add char to the current string and keep track of current level
            current_level = formatted_char.m_level;
            utf8::append(formatted_char.m_chr, std::back_inserter(current_str));
        }

        // draw the rest of the string
        if (current_str.length() != 0) {
            if (!style_stack.positions.empty())
                pos += style_stack.positions.back().value;
            font.render(current_str, style_stack.get_font_style(), &shader, mvp, &pos, actual_rendering);
        }
    };


    for (auto& element : elements) {
        std::visit(
            detail::text_formatter::overloaded{
                render_icon,
                render_text
            }
            , element);
    }
    if (!m_icon_batch.empty()) {
        auto icon_mvp = xd::translate(mvp, xd::vec3(0, -m_icon_size.y, 0));
        m_icon_batch.draw(xd::shader_uniforms{icon_mvp});
        m_icon_batch.clear();
    }

    return pos;
}

void xd::text_formatter::parse(const std::string& text, std::list<detail::text_formatter::token>& tokens)
{
    // parser state variables
    std::list<std::string> open_decorators;
    std::string current_text;
    std::unordered_map<std::string, std::string> escape_delims = {
        { m_decorator_open_delim, m_decorator_open_escape_delim },
        { m_decorator_close_delim, m_decorator_close_escape_delim },
        { m_variable_open_delim, m_variable_open_escape_delim },
        { m_variable_close_delim, m_variable_close_escape_delim }
    };

    // parse!
    std::string::const_iterator start = text.begin(), end = text.end();
    while (start != end) {
        // Skip escape delims
        auto skip = false;
        for (auto& [delim, escape_delim] : escape_delims) {
            if (detail::text_formatter::safe_equal(escape_delim, start, end)) {
                std::string::const_iterator delim_start = delim.begin(), delim_end = delim.cend();
                utf8::append(utf8::next(delim_start, delim_end), std::back_inserter(current_text));
                utf8::advance(start, utf8::distance(escape_delim.begin(), escape_delim.end()), end);
                skip = true;
                break;
            }
        }
        if (skip) continue;

        // check if we have an opening decorator delimiter
        if (detail::text_formatter::safe_equal(m_decorator_open_delim, start, end)) {
            // push current text node
            if (current_text.length() != 0) {
                detail::text_formatter::token_text tok;
                tok.text = current_text;
                tokens.push_back(tok);
                current_text = "";
            }

            // consume the delimiter
            utf8::advance(start, utf8::distance(m_decorator_open_delim.begin(), m_decorator_open_delim.end()), end);

            // check is this opening or closing tag
            bool open_decorator;
            if (detail::text_formatter::safe_equal(m_decorator_terminate_delim, start, end)) {
                // closing tag
                utf8::advance(start, utf8::distance(m_decorator_terminate_delim.begin(), m_decorator_terminate_delim.end()), end);
                open_decorator = false;
            } else {
                open_decorator = true;
            }

            // get the decorator name
            std::string decorator_name;
            while (start != end && (*start != '=' || !open_decorator)
                    && (!detail::text_formatter::safe_equal(m_decorator_terminate_delim, start, end) || !open_decorator)
                    && !detail::text_formatter::safe_equal(m_decorator_close_delim, start, end)) {
                utf8::append(utf8::next(start, end), std::back_inserter(decorator_name));
            }

            // check if we have parameters
            text_decorator_args args;
            if (start != end && *start == '=' && open_decorator) {
                // consume the assignment sign
                ++start;

                // parse arguments
                std::string arg;
                while (start != end
                        && !detail::text_formatter::safe_equal(m_decorator_close_delim, start, end)
                        && !detail::text_formatter::safe_equal(m_decorator_terminate_delim, start, end)) {
                    if (*start == ',') {
                        // push and reset the argument
                        args.m_args.push_back(arg);
                        arg = "";
                        ++start;
                    } else {
                        // aggregate the argument
                        utf8::append(utf8::next(start, end), std::back_inserter(arg));
                    }
                }

                // push the last arg
                args.m_args.push_back(arg);
            }

            // Check for self-closing decorator
            auto self_closing = false;
            if (start != end && open_decorator && detail::text_formatter::safe_equal(m_decorator_terminate_delim, start, end)) {
                utf8::advance(start, utf8::distance(m_decorator_terminate_delim.begin(), m_decorator_terminate_delim.end()), end);
                self_closing = true;
            }

            // closing delimiter not found
            if (start == end) {
                throw text_formatter_parse_exception(text, "closing delimiter for decorator \""+decorator_name+"\" not found");
            }

            // make sure it's registered
            auto is_icon = decorator_name == "icon";
            decorator_list_t::iterator decorator_pos = m_decorators.find(decorator_name);
            if (!is_icon && decorator_pos == m_decorators.end()) {
                throw text_formatter_parse_exception(text, "decorator \""+decorator_name+"\" not registered");
            }

            // tag closed
            utf8::advance(start, utf8::distance(m_decorator_close_delim.begin(), m_decorator_close_delim.end()), end);

            // push the tag in tokens
            if (open_decorator) {
                // push the open decorator in open decorators list
                open_decorators.push_back(decorator_name);

                // push the token
                if (is_icon) {
                    tokens.push_back(detail::text_formatter::token_icon{args.get<int>(0)});
                } else {
                    detail::text_formatter::token_open_decorator tok;
                    tok.name = decorator_name;
                    tok.args = args;
                    tok.callback = decorator_pos->second;
                    tokens.push_back(tok);
                }
            }

            if (!open_decorator || self_closing) {
                // check that tags are closed in correct order
                if (open_decorators.size() == 0) {
                    throw text_formatter_parse_exception(text, "decorator \""+decorator_name+"\" closed without opening tag");
                } else if (open_decorators.back() != decorator_name) {
                    throw text_formatter_parse_exception(text, "decorator \""+decorator_name+"\" closed out of order, expected \""+open_decorators.back()+"\"");
                }

                // pop the open decorator
                open_decorators.pop_back();

                // push the token
                if (!is_icon) {
                    detail::text_formatter::token_close_decorator tok;
                    tok.name = decorator_name;
                    tokens.push_back(tok);
                }
            }

            continue;
        }

        // check if we have an opening variable delimiter
        if (detail::text_formatter::safe_equal(m_variable_open_delim, start, end)) {
            // push current text node
            if (current_text.length() != 0) {
                detail::text_formatter::token_text tok;
                tok.text = current_text;
                tokens.push_back(tok);
                current_text = "";
            }

            // consume the delimiter
            utf8::advance(start, utf8::distance(m_variable_open_delim.begin(), m_variable_open_delim.end()), end);

            // get the tag name
            std::string variable_name;
            while (start != end && !detail::text_formatter::safe_equal(m_variable_close_delim, start, end)) {
                utf8::append(utf8::next(start, end), std::back_inserter(variable_name));
            }

            // closing delimiter not found
            if (start == end) {
                throw text_formatter_parse_exception(text, "closing delimiter for variable \""+variable_name+"\" not found");
            }

            // make sure it's registered
            variable_list_t::iterator variable_pos = m_variables.find(variable_name);
            if (variable_pos == m_variables.end()) {
                throw text_formatter_parse_exception(text, "variable \""+variable_name+"\" not registered");
            }

            // tag closed
            utf8::advance(start, utf8::distance(m_variable_close_delim.begin(), m_variable_close_delim.end()), end);

            // push the token
            detail::text_formatter::token_variable tok;
            tok.name = variable_name;
            tok.callback = variable_pos->second;
            tokens.push_back(tok);

            continue;
        }

        utf8::append(utf8::next(start, end), std::back_inserter(current_text));
    }

    // check if any decorators were left open
    if (open_decorators.size() != 0) {
        throw text_formatter_parse_exception(text, "unclosed decorator \""+open_decorators.back()+"\"");
    }

    // push the last text node if one exists at this point
    if (current_text.length() != 0) {
        detail::text_formatter::token_text tok;
        tok.text = current_text;
        tokens.push_back(tok);
    }
}
