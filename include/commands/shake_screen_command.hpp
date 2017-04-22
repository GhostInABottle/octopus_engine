#ifndef HPP_SHAKE_SCREEN_COMMAND
#define HPP_SHAKE_SCREEN_COMMAND

#include "../command.hpp"

class Game;

class Shake_Screen_Command : public Command {
public:
	Shake_Screen_Command(Game& game, float strength, float speed, long duration);
	void execute() {}
	bool is_complete() const;
private:
	Game& game;
	long start_time;
	long duration;
};



#endif
