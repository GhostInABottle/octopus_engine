#ifndef HPP_WAIT_COMMAND
#define HPP_WAIT_COMMAND

#include "../command.hpp"

class Game;

class Wait_Command : public Simulatable_Command {
public:
	Wait_Command(Game& game, long duration, int start = -1);
	void execute() {}
	bool is_complete() const;
	bool is_complete(int ticks) const;
private:
	Game& game;
	long start_time;
	long duration;
};



#endif
