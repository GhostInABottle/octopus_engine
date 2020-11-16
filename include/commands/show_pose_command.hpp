#ifndef HPP_SHOW_POSE_COMMAND
#define HPP_SHOW_POSE_COMMAND

#include <string>
#include "../direction.hpp"
#include "../command.hpp"

struct Sprite_Holder;

class Show_Pose_Command : public Command {
public:
    Show_Pose_Command(Sprite_Holder* holder, const std::string& pose_name,
        const std::string& state = "", Direction dir = Direction::NONE);
    void execute() override {}
    bool is_complete() const override;
    void pause() override;
    void resume() override;
private:
    Sprite_Holder* holder;
};

#endif