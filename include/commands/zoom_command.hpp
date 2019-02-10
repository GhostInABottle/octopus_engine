#ifndef HPP_ZOOM_COMMAND
#define HPP_ZOOM_COMMAND

#include "../command.hpp"

class Game;

class Zoom_Command : public Command {
public:
    Zoom_Command(Game& game, float magnification, long duration);
    void execute() override;
    bool is_complete() const override;
private:
    Game& game;
    float old_magnification;
    float new_magnification;
    long start_time;
    long duration;
    bool complete;
};

#endif