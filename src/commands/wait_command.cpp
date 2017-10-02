#include "../../include/commands/wait_command.hpp"
#include "../../include/game.hpp"

Wait_Command::Wait_Command(Game& game, long duration, int start)
	: game(game), duration(duration),
	start_time(start < 0 ? game.ticks() : start) {}

bool Wait_Command::is_complete() const {
	return is_complete(game.ticks());
}

bool Wait_Command::is_complete(int ticks) const {
	return stopped || ticks > start_time + duration;
}
