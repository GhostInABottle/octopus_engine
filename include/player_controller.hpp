#ifndef HPP_PLAYER_CONTROLLER
#define HPP_PLAYER_CONTROLLER

#include <string>
#include "map_object.hpp"

class Game;

class Player_Controller : public xd::logic_component<Map_Object>
{
public:
    explicit Player_Controller(Game& game);
    void update(Map_Object& object);
private:
    Game& game;
    std::string action_button;
    int last_collision_check;
    int collision_check_delay;
    int edge_tolerance_pixels;
    void process_collision(Map_Object& object, Collision_Record collision, Collision_Type type, bool moved, bool action_pressed);
};

#endif
