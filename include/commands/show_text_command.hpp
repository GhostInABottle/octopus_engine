#ifndef HPP_SHOW_TEXT_COMMAND
#define HPP_SHOW_TEXT_COMMAND

#include <vector>
#include <memory>
#include "../command.hpp"

class Game;
class Map_Object;
class Canvas;

class Show_Text_Command : public Command {
public:
	Show_Text_Command(Game& game, Map_Object* object, const std::string& text, long duration = -1);
	Show_Text_Command(Game& game, Map_Object* object, std::vector<std::string> choices, const std::string& text = "");
	void execute();
	bool is_complete() const;
	int choice_index() const { return selected_choice; }
	std::string full_text() const;
private:
	void init();
	void update_choice();
	Game& game;
	Map_Object* object;
	std::shared_ptr<Canvas> canvas;
	std::string text;
	std::vector<std::string> choices;
	bool complete;
	bool was_disabled;
	unsigned int  selected_choice;
	unsigned int current_choice;
	long start_time;
	long duration;
};

#endif
