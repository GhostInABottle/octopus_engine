#ifndef HPP_CANVAS_UPDATER
#define HPP_CANVAS_UPDATER

#include "../map.hpp"
#include "../xd/entity.hpp"

class Canvas_Updater : public xd::logic_component<Map> {
public:
    void update(Map& map);
};

#endif
