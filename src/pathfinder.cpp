#include "../include/configurations.hpp"
#include "../include/map/map.hpp"
#include "../include/map/map_object.hpp"
#include "../include/pathfinder.hpp"
#include "../include/utility/direction.hpp"

namespace detail {
    static bool debug_mode = false;
    static void show_test_tile(Map& map, int x, int y, const std::string& type) {
        auto pos = xd::vec2(x * map.get_tile_width(), y * map.get_tile_height());
        std::string name = std::to_string(x) + ", " + std::to_string(y);
        auto obj = map.get_object(name);
        if (!obj) {
            std::string sprite = Configurations::get<std::string>("debug.pathfinding-sprite");
            obj = map.add_new_object(name, sprite, pos);
        }
        obj->set_pose(type);
        obj->set_opacity(0.25);
        obj->set_passthrough(true);
    }
}

Pathfinder::Pathfinder(Map& map, Map_Object& object,
    xd::vec2 dest, int range, bool close, Collision_Check_Type check_type) :
        map(map),
        object(object),
        goal_node(map.get_tile_width(), map.get_tile_height(), dest),
        start_node(map.get_tile_width(), map.get_tile_height(), object.get_real_position()),
        range(range),
        get_close(close),
        original_goal(map.get_tile_width(), map.get_tile_height(), dest),
        found(false),
        check_type(check_type),
        nearest_node(map.get_tile_width(), map.get_tile_height()) {
    nearest_node.h = -1;
    start_node.h = distance(start_node.tile_pos(), goal_node.tile_pos());
    open_list.push(start_node);
    detail::debug_mode = !Configurations::get<std::string>("debug.pathfinding-sprite").empty();
}

bool Pathfinder::Node::in_range(Node& other, int range) const {
    auto this_pos = tile_pos();
    auto other_pos = other.tile_pos();
    int abs_sx = std::abs(this_pos.x - other_pos.x);
    int abs_sy = std::abs(this_pos.y - other_pos.y);
    return abs_sx + abs_sy <= range;
}

void Pathfinder::calculate_path() {
    auto goal_pos = goal_node.tile_pos();
    if (map.tile_passable(goal_pos.x, goal_pos.y) || get_close || range > 0) {
        while (!open_list.empty()) {
            // Get the node with lowest cost and add it to the closed list
            Node current_node = open_list.pop();
            closed_list[current_node.tile_pos()] = current_node;
            if (current_node == goal_node ||
                    (range > 0 && goal_node.in_range(current_node, range))) {
                // We reached the goal, exit the loop
                original_goal = goal_node;
                goal_node = current_node;
                nearest_node = goal_node;
                found = true;
                break;
            } else { // if not goal
                // Keep track of the node with the lowest cost so far
                if (current_node.h < nearest_node.h || nearest_node.h < 1 ||
                        (current_node.h == nearest_node.h &&
                        current_node.tile_pos() == nearest_node.tile_pos()))
                    nearest_node = current_node;

                // Check if we can add adjacent nodes to open list
                auto adjacent_nodes = get_adjacent_nodes(current_node);
                for (auto& adj_node : adjacent_nodes) {
                    if (skip_node(adj_node))
                        continue;   // Node already exists in one of the lists
                    open_list.push(adj_node);
                }
            }
        }
    }
    // If no path is found, see if we can get close to goal
    if (!found && get_close && nearest_node.h > 0) {
        goal_node = nearest_node;
        goal_node.parent = nullptr;
        open_list.push(nearest_node);
        calculate_path();
    }
}

void Pathfinder::add_node(std::vector<Node>& nodes, xd::vec2 pos, Node& parent) {
    auto tile_height = map.get_tile_height();
    auto tile_width = map.get_tile_width();
    auto tile_pos = static_cast<xd::ivec2>(pos) / tile_width;
    if (!object.is_passthrough() && !map.tile_passable(tile_pos.x, tile_pos.y)) {
        if (detail::debug_mode)
            detail::show_test_tile(map, tile_pos.x, tile_pos.y, "TPass");
        return;
    }
    auto dir = facing_direction(parent.pos, pos, true);
    auto obj_pos = parent.pos;
    auto collision = map.passable(object, dir, obj_pos, static_cast<float>(tile_width), check_type);
    if (collision.passable()) {
        int g = parent.g + 1;
        int h = distance(tile_pos, goal_node.tile_pos());
        // Penalize frequent path changes when moving diagonally
        if (is_diagonal(dir) && parent.parent) {
            auto p_dir = facing_direction(parent.parent->pos, parent.pos, true);
            if (dir != p_dir)
                h++;
        }
        Node* parent_address = &closed_list[parent.tile_pos()];
        nodes.emplace_back(tile_width, tile_height, pos, parent_address, g, h);
        if (detail::debug_mode)
            detail::show_test_tile(map, tile_pos.x, tile_pos.y, "Pass");
    } else if (detail::debug_mode) {
        detail::show_test_tile(map, tile_pos.x, tile_pos.y, "Block");
    }
}

std::vector<Pathfinder::Node> Pathfinder::get_adjacent_nodes(Node& node) {
    std::vector<Node> nodes;
    xd::vec2 new_pos;
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i != 0 || j != 0) {
                new_pos = xd::ivec2(node.pos.x + i * map.get_tile_width(),
                    node.pos.y + j * map.get_tile_height());
                add_node(nodes, new_pos, node);
            }
        }
    }

    return nodes;
}

bool Pathfinder::skip_node(const Node& node) {
    bool skip = false;
    // Check the open list
    int copy = open_list.index(node);
    if (copy != -1) {
        // Swap nodes if new node is better than existing copy
        if (open_list[copy].cost() >= node.cost()) {
            open_list[copy] = node;
            open_list.update();
        }
        skip = true;
    }
    // Check the closed list
    auto node_pos = node.tile_pos();
    if (closed_list.find(node_pos) != closed_list.end()) {
        if (closed_list[node_pos].cost() <= node.cost())
            skip = true;
        else
            closed_list[node_pos] = node;
    }
    return skip;
}

std::deque<Direction> Pathfinder::generate_path() {
    std::deque<Direction> path;
    if (!found)
        return path;
    // Generate path by starting from goal and following parents
    Node* node = &goal_node;
    while (node->parent) {
        auto direction = facing_direction(node->parent->pos, node->pos, true);
        path.push_front(direction);
        node = node->parent;
    }
    return path;
}
