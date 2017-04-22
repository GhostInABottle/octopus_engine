#ifndef HPP_TINT_SCREEN_COMMAND
#define HPP_TINT_SCREEN_COMMAND

#include <xd/graphics/types.hpp>
#include "../command.hpp"

class Game;

class Tint_Screen_Command : public Command {
public:
	Tint_Screen_Command(Game& game, xd::vec4 color, long duration);
	void execute();
	bool is_complete() const;
private:
	Game& game;
	xd::vec4 old_color;
	xd::vec4 new_color;
	long start_time;
	long duration;
};

#endif
