#ifndef HPP_OPACITY_HOLDER
#define HPP_OPACITY_HOLDER

// An interface for objects that have opacity
class Opacity_Holder {
public:
    virtual ~Opacity_Holder() {}
    virtual float get_opacity() const = 0;
    virtual void set_opacity(float new_opacity) = 0;
};

#endif
