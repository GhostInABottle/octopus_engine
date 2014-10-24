#ifndef HPP_PLAYER_CONTROLLER
#define HPP_PLAYER_CONTROLLER

#include "map_object.hpp"

class Game;

class Player_Controller : public xd::logic_component<Map_Object>
{
public:
    Player_Controller(Game& game);
    void update(Map_Object& object);
private:
    Game& game;
};

#endif
