#ifndef HPP_SPRITE_HOLDER
#define HPP_SPRITE_HOLDER

#include <string>
#include "../direction.hpp"

class Game;
class Sprite;

class Sprite_Holder {
public:
    virtual ~Sprite_Holder() = 0;
    virtual void set_pose(const std::string& pose_name,
            const std::string& state, Direction direction);
    virtual void reset();
    // Set sprite
    virtual void set_sprite(Game& game, const std::string& filename, const std::string& pose_name = "") = 0;
    virtual Sprite* get_sprite() = 0;
    virtual const Sprite* get_sprite() const = 0;
    std::string get_sprite_filename();
};

#endif
