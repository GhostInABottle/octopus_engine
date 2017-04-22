#include "../../include/commands/move_object_to_command.hpp"
#include "../../include/map_object.hpp"
#include "../../include/map.hpp"
#include "../../include/game.hpp"
#include "../../include/pathfinder.hpp"

Move_Object_To_Command::Move_Object_To_Command(Map& map, Map_Object& object,
	float x, float y, Collision_Check_Types check_type, bool keep_trying)
	: map(map), object(object), destination(x, y), pixels(0.0f),
	check_type(check_type), keep_trying(keep_trying), last_attempt_time(0),
	blocked(false), nearest(nullptr) {}

Move_Object_To_Command::~Move_Object_To_Command() {}

void Move_Object_To_Command::execute() {
	if ((blocked || path.empty()) && keep_trying) {
		object.update_state("FACE");
		int time_passed = map.get_game().ticks() - last_attempt_time;
		if ((map.get_objects_moved() && time_passed > 1000) || time_passed > 5000) {
			init();
			map.set_objects_moved(false);
			blocked = false;
		}
		return;
	}
	if (path.empty()) {
		return;
	}
	int index = static_cast<int>(pixels) / map.get_tile_width();
	int max_index = static_cast<int>(path.size() - 1);
	if (index > max_index && !is_complete()) {
		blocked = true;
		return;
	}
	// TODO: Temp fix for back alley collision bug
	if (check_type == Collision_Check_Types::TILE)
		object.set_passthrough(true);
	auto collision = object.move(path[index], object.get_speed(), check_type);
	if (check_type == Collision_Check_Types::TILE)
		object.set_passthrough(false);
	if (collision.passable())
		pixels += object.get_speed();
	else
		blocked = true;
}

void Move_Object_To_Command::init() {
	Pathfinder finder(map, object, destination, 0, true, check_type);
	if (nearest)
		finder.nearest() = *nearest;
	finder.calculate_path();
	if (finder.nearest().h > 0 &&
		(!nearest || finder.nearest().h < nearest->h)) {
		nearest.reset(new Node(finder.nearest()));
		nearest->parent = nullptr;
	}
	path = finder.generate_path();
	pixels = 0.0f;
	last_attempt_time = map.get_game().ticks();
}

bool Move_Object_To_Command::is_complete(int ticks) const {
	auto pos = object.get_real_position();
	bool complete = object.is_stopped() || (!keep_trying && path.empty()) ||
		(std::abs(pos.x - destination.x) < 8.0f &&
			std::abs(pos.y - destination.y) < 8.0f);
	if (complete)
		object.update_state("FACE");
	return complete;
}