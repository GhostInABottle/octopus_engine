#ifndef H_XD_GRAPHICS_DETAIL_TEXT_FORMATTER
#define H_XD_GRAPHICS_DETAIL_TEXT_FORMATTER

#include "../font.hpp"
#include <variant>
#include <list>

namespace xd { namespace detail { namespace text_formatter {

    // text decorator visitor
    class token_decorator;
    class formatted_element_renderer;

    // tokens
    struct token_text;
    struct token_variable;
    struct token_icon;
    struct token_open_decorator;
    struct token_close_decorator;

    // variant token
    typedef std::variant<
        token_text,
        token_variable,
        token_icon,
        token_open_decorator,
        token_close_decorator
    > token;

    // nested value
    template <typename T>
    struct nested_value
    {
        T value;
        int level;
    };

    // no value attached
    template <>
    struct nested_value<void>
    {
        int level;
    };

    // typedefs for styles
    typedef nested_value<glm::vec4> nested_color;
    typedef nested_value<int> nested_size;
    typedef nested_value<float> nested_alpha;
    typedef nested_value<std::string> nested_type;
    typedef nested_value<font_shadow> nested_shadow;
    typedef nested_value<font_outline> nested_outline;
    typedef nested_value<glm::vec2> nested_position;
    typedef nested_value<float> nested_letter_spacing;
    typedef nested_value<bool> nested_force_autohint;
    typedef nested_value<void> nested_void;

    // state changes
    struct state_change_push_color : nested_color {};
    struct state_change_push_alpha : nested_alpha {};
    struct state_change_push_size : nested_size {};
    struct state_change_push_type : nested_type {};
    struct state_change_push_shadow : nested_shadow {};
    struct state_change_push_outline : nested_outline {};
    struct state_change_push_position : nested_position {};
    struct state_change_push_letter_spacing : nested_letter_spacing {};
    struct state_change_push_force_autohint : nested_force_autohint {};
    struct state_change_pop_color : nested_void {};
    struct state_change_pop_alpha : nested_void {};
    struct state_change_pop_size : nested_void {};
    struct state_change_pop_type : nested_void {};
    struct state_change_pop_shadow : nested_void {};
    struct state_change_pop_outline : nested_void {};
    struct state_change_pop_position : nested_void {};
    struct state_change_pop_letter_spacing : nested_void {};
    struct state_change_pop_force_autohint : nested_void {};

    // variant state change
    typedef std::variant<
        state_change_push_color,
        state_change_push_alpha,
        state_change_push_size,
        state_change_push_type,
        state_change_push_shadow,
        state_change_push_outline,
        state_change_push_position,
        state_change_push_letter_spacing,
        state_change_push_force_autohint,
        state_change_pop_color,
        state_change_pop_alpha,
        state_change_pop_size,
        state_change_pop_type,
        state_change_pop_shadow,
        state_change_pop_outline,
        state_change_pop_position,
        state_change_pop_letter_spacing,
        state_change_pop_force_autohint
    > state_change;

    // token list
    typedef std::list<token> token_list;

    // state change list
    typedef std::list<state_change> state_change_list;

} } }

#endif
