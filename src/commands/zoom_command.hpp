#ifndef HPP_ZOOM_COMMAND
#define HPP_ZOOM_COMMAND

#include "timed_command.hpp"

class Game;

class Zoom_Command : public Timed_Command {
public:
    Zoom_Command(Game& game, float magnification, long duration);
    void execute() override;
    bool is_complete() const override;
private:
    float old_magnification;
    float new_magnification;
    bool complete;
};

#endif