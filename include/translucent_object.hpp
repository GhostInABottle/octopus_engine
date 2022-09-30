#ifndef HPP_TRANSLUCENT_OBJECT
#define HPP_TRANSLUCENT_OBJECT

// An interface for objects that have opacity
class Translucent_Object {
public:
	virtual ~Translucent_Object() {}
	virtual float get_opacity() const = 0;
	virtual void set_opacity(float new_opacity) = 0;
};

#endif
