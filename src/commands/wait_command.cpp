#include "../../include/commands/wait_command.hpp"
#include "../../include/game.hpp"

Wait_Command::Wait_Command(Game& game, int duration, int start)
    : Timed_Command(game, duration, start) {}

bool Wait_Command::is_complete() const {
    return is_complete(game.ticks());
}

bool Wait_Command::is_complete(int ticks) const {
    return stopped || is_done(ticks);
}
