#include "text_options.hpp"
#include "../map/map_object.hpp"

Text_Options& Text_Options::set_object(Map_Object* object) {
    this->object = object;
    position = object->get_text_position();
    set_position_type(Text_Position_Type::CENTERED_X
        | Text_Position_Type::BOTTOM_Y
        | Text_Position_Type::ALWAYS_VISIBLE);
    return *this;
}
