#include "../../include/commands/move_object_to_command.hpp"
#include "../../include/map_object.hpp"
#include "../../include/map.hpp"
#include "../../include/game.hpp"
#include "../../include/pathfinder.hpp"
#include "../../include/direction.hpp"
#include "../../include/direction_utilities.hpp"
#include "../../include/xd/graphics/types.hpp"
#include <deque>
#include <string>


struct Move_Object_To_Command::Impl {
    Impl(Map& map, Map_Object& object, float x, float y,
        Collision_Check_Types check_type, bool keep_trying)
        : map(map), object(object), destination(x, y), path_found(false), pixels(0.0f),
        check_type(check_type), keep_trying(keep_trying), last_attempt_time(0),
        blocked(false), complete(false), nearest(nullptr), old_state(object.get_state()) {}
    Map& map;
    Map_Object& object;
    xd::vec2 destination;
    std::deque<Direction> path;
    bool path_found;
    float pixels;
    bool keep_trying;
    int last_attempt_time;
    bool blocked;
    bool complete;
    Collision_Check_Types check_type;
    std::unique_ptr<Pathfinder::Node> nearest;
    std::string old_state;
    // Setup the pathfinder
    void init() {
        complete = false;
        path_found = false;
        Pathfinder finder(map, object, destination, 0, true, check_type);
        if (nearest)
            finder.nearest() = *nearest;
        finder.calculate_path();
        if (finder.nearest().h > 0 &&
            (!nearest || finder.nearest().h < nearest->h)) {
            nearest.reset(new Pathfinder::Node(finder.nearest()));
            nearest->parent = nullptr;
        }
        path = finder.generate_path();
        path_found = finder.is_found();
        pixels = 0.0f;
        last_attempt_time = map.get_game().ticks();
    }
    // Move object in direction
    Collision_Record move_object(Direction dir) {
        if (check_type == Collision_Check_Types::TILE)
            object.set_passthrough(true);
        auto collision = object.move(dir, object.get_speed(), check_type);
        if (check_type == Collision_Check_Types::TILE)
            object.set_passthrough(false);
        return collision;
    }
    // Called every frame
    void execute(bool stopped) {
        if ((blocked || !path_found) && keep_trying) {
            object.update_state(old_state);
            int time_passed = map.get_game().ticks() - last_attempt_time;
            if ((map.get_objects_moved() && time_passed > 1000) || time_passed > 5000) {
                init();
                map.set_objects_moved(false);
                blocked = false;
            }
            return;
        }

        if (!path_found) {
            return;
        }

        auto check_completion = [&](bool is_stopped) {
            auto pos = object.get_real_position();
            return is_stopped || object.is_stopped() || (!path_found && !keep_trying) ||
                (check_close(pos.x, destination.x, 1) && check_close(pos.y, destination.y, 1));
        };

        int index = static_cast<int>(pixels) / map.get_tile_width();
        int max_index = static_cast<int>(path.size() - 1);

        if (index <= max_index) {
            auto collision = move_object(path[index]);
            if (collision.passable())
                pixels += object.get_speed();
            else
                blocked = true;

            if (complete = check_completion(stopped)) {
                object.update_state(old_state);
            }

            return;
        }

        if (!(complete = check_completion(stopped))) {
            // Try to manually move to the destination
            auto pos = object.get_real_position();
            auto tile_width = static_cast<float>(map.get_tile_width());
            auto tile_height = static_cast<float>(map.get_tile_width());
            auto within_tile = (std::abs(pos.x - destination.x) <= tile_width
                    && std::abs(pos.y - destination.y) <= tile_height);
            if (within_tile) {
                move_object(facing_direction(pos, destination, true));
            } else {
                blocked = true;
            }
        }

        if (complete = check_completion(stopped)) {
            object.update_state(old_state);
        }
    }
    // Is movement complete
    bool is_complete(bool stopped) const {
        return complete;
    }
};

Move_Object_To_Command::Move_Object_To_Command(Map& map, Map_Object& object,
    float x, float y, Collision_Check_Types check_type, bool keep_trying)
    :  pimpl(new Impl(map, object, x, y, check_type, keep_trying)) {}

Move_Object_To_Command::~Move_Object_To_Command() {}


void Move_Object_To_Command::execute() {
    pimpl->execute(stopped);
}

bool Move_Object_To_Command::is_complete() const {
    return pimpl->is_complete(stopped);
}