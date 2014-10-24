#ifndef HPP_CANVAS_UPDATER
#define HPP_CANVAS_UPDATER

#include <xd/config.hpp>
#include <xd/entity.hpp>
#include "map.hpp"

class Canvas_Updater : public xd::logic_component<Map> {
public:
    void update(Map& map);
};

#endif
