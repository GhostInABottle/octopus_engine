#ifndef HPP_PLAYER_CONTROLLER
#define HPP_PLAYER_CONTROLLER

#include "map_object.hpp"

class Game;

class Player_Controller : public xd::logic_component<Map_Object>
{
public:
    explicit Player_Controller(Game& game);
    void update(Map_Object& object);
private:
    Game& game;
    void process_collision(Map_Object& object, Collision_Record collision, Collision_Types type, bool action_pressed);
};

#endif
