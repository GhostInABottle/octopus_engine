#ifndef HPP_UPDATE_LAYER_COMMAND
#define HPP_UPDATE_LAYER_COMMAND

#include "../command.hpp"

class Game;
struct Layer;

class Update_Layer_Command : public Command {
public:
	Update_Layer_Command(Game& game, Layer& layer, float opacity, long duration);
	void execute();
	bool is_complete() const;
private:
	Layer& layer;
	Game& game;
	float old_opacity;
	float new_opacity;
	long start_time;
	long duration;
};


#endif
