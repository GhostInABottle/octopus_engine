#ifndef HPP_MOVE_CAMERA_COMMAND
#define HPP_MOVE_CAMERA_COMMAND

#include <xd/graphics/types.hpp>
#include "../direction.hpp"
#include "../command.hpp"

class Camera;
class Map_Object;

class Move_Camera_Command : public Command {
public:
    Move_Camera_Command(Camera& camera, float x, float y, float speed);
    Move_Camera_Command(Camera& camera, Direction dir, float pixels, float speed);
    void execute();
    bool is_complete() const;
private:
    Camera& camera;
    const Map_Object* camera_object;
    xd::vec2 direction;
    float pixels;
    float speed;
};

#endif