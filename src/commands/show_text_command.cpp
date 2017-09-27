#include "../../include/commands/show_text_command.hpp"
#include "../../include/game.hpp"
#include "../../include/camera.hpp"
#include "../../include/canvas.hpp"
#include "../../include/map_object.hpp"
#include "../../include/utility.hpp"
#include "../../include/configurations.hpp"

Show_Text_Command::Show_Text_Command(Game& game, Map_Object* object,
	const std::string& text, long duration) :
	Show_Text_Command(game, text_position(object), std::vector<std::string>{},
		text, duration, false, Text_Position_Type::CENTERED_X | Text_Position_Type::BOTTOM_Y) {}

Show_Text_Command::Show_Text_Command(Game& game, Map_Object* object,
	std::vector<std::string> choices, const std::string& text) :
	Show_Text_Command(game, text_position(object), choices, text, -1, false,
		Text_Position_Type::CENTERED_X | Text_Position_Type::BOTTOM_Y) {}

Show_Text_Command::Show_Text_Command(Game& game, xd::vec2 position,
	std::vector<std::string> choices, const std::string& text,
	long duration, bool center, Text_Position_Type pos_type) :
		game(game), position(position), choices(choices), text(text),
		duration(duration), start_time(game.ticks()), complete(false) {
	selected_choice = 0;
	current_choice = 0;
	auto full = full_text();
	// Estimate text size by stripping out tags
	auto clean_text = full;
	int start = 0;
	while ((start = clean_text.find_first_of('{')) != std::string::npos) {
		int end = clean_text.find_first_of('}', start);
		if (end != std::string::npos) {
			clean_text.erase(start, end - start + 1);
		}
	}
	auto text_lines = split(clean_text, "\n");
	if (text_lines.empty())
		text_lines.push_back("");
	// Find longest line to center the text at the right position
	auto longest_line = text_lines[0];
	for (auto& line : text_lines) {
		if (line.size() > longest_line.size())
			longest_line = line;
	}
	// Set text position based on size estimation
	float char_height = 8;
	float text_width = game.get_font()->get_width(longest_line,
		xd::font_style(xd::vec4(1.0f, 1.0f, 1.0f, 1.0f), 8)
		.force_autohint(true));
	float text_height = char_height * text_lines.size();
	auto pos = position - game.get_camera()->get_position();
	if ((pos_type & Text_Position_Type::BOTTOM_Y) != Text_Position_Type::NONE) {
		pos.y -= text_height;
	}
	if (center) {
		pos.x = game.game_width / 2 - text_width / 2;
	}
	else if ((pos_type & Text_Position_Type::CENTERED_X) != Text_Position_Type::NONE) {
		pos.x -= text_width / 2;
	}
	// Make sure text fits on the screen
	if (pos.x + text_width > Game::game_width - 10)
		pos.x = static_cast<float>(Game::game_width - text_width - 10);
	if (pos.x < 10.0f)
		pos.x = 10.0f;
	if (pos.y + text_height > Game::game_height - 10)
		pos.y = static_cast<float>(Game::game_height - text_height * 2);
	if (pos.y < 25.0f)
		pos.y = 25.0f;
	// Create the text canvas and show it
	canvas = std::make_shared<Canvas>(game, pos, full);
	game.add_canvas(canvas);
	was_disabled = game.get_player()->is_disabled();
	game.get_player()->set_disabled(true);
	canvas->set_visible(true);
}

void Show_Text_Command::execute() {
	if (complete)
		return;
	if (duration > -1) {
		complete = game.ticks() > start_time + duration;
	}
	else {
		static std::string action_button =
			Configurations::get<std::string>("controls.action-button");
		complete = game.triggered_once(action_button);
		if (!choices.empty()) {
			if (complete)
				selected_choice = current_choice;
			else
				update_choice();
		}
	}
}

void Show_Text_Command::update_choice() {
	unsigned int old_choice = current_choice;
	if (game.triggered("down"))
		current_choice = (current_choice + 1) % choices.size();
	if (game.triggered("up"))
		current_choice = (current_choice + choices.size() - 1) % choices.size();
	if (old_choice != current_choice)
		canvas->set_text(full_text());
}

xd::vec2 Show_Text_Command::text_position(Map_Object* object) {
	return object->get_position() + xd::vec2(16, 0);
}

bool Show_Text_Command::is_complete() const {
	if (complete && canvas->is_visible()) {
		canvas->set_visible(false);
		game.get_player()->set_disabled(was_disabled);
		game.remove_canvas(canvas);
	}
	return complete;
}

std::string Show_Text_Command::full_text() const {
	std::string result = text;
	std::string color_prefix = "{color=";
	for (unsigned int i = 0; i < choices.size(); ++i) {
		if (!result.empty())
			result += "\n";
		std::string choice_text = choices[i];
		bool replaced_color = false;
		// Add color for selected choice
		if (i == current_choice) {
			auto start = choices[i].find(color_prefix);
			// Strip existing outermost color, we want green to take precedence
			if (start == 0) {
				auto end = choices[i].find("}");
				choice_text.replace(color_prefix.length(),
					end - color_prefix.length(), "green");
				replaced_color = true;
			}
			else
				result += "{color=green}";
		}
		// Add padding before choices if header text was specified
		if (!text.empty())
			result += "  ";
		result += "- " + choice_text;
		if (i == current_choice && !replaced_color)
			result += "{/color}";
	}
	return result;
}
