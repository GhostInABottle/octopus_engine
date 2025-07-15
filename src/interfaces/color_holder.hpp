#ifndef HPP_COLOR_HOLDER
#define HPP_COLOR_HOLDER

#include "../xd/glm.hpp"

// An interface for objects that have color
class Color_Holder {
public:
    virtual ~Color_Holder() {}
    virtual xd::vec4 get_color() const = 0;
    virtual void set_color(xd::vec4 new_color) = 0;
};

#endif