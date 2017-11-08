#ifndef HPP_PATHFINDER
#define HPP_PATHFINDER

#include <cmath>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <deque>
#include <vector>
#include "direction.hpp"
#include "collision_check_types.hpp"

// Hash specialization for ivec2
namespace std {
    template <>
    struct hash<xd::ivec2> {
       std::hash<int> xh;
       std::hash<int> yh;
        size_t operator()(xd::ivec2 v) const throw() { 
                return xh(v.x) ^ yh(v.y);
        }
    };
}

struct Node {
    int tile_width, tile_height;
    xd::vec2 pos;
    Node* parent;
    // cost of getting to this node
    int g;
    // distance to goal (heuristic)
    int h;
    Node(
        int tile_width = 1,
        int tile_height = 1,
        xd::vec2 pos = xd::vec2(),
        Node* parent = nullptr,
        int g = 0,
        int h = 0) :
        tile_width(tile_width),
        tile_height(tile_height),
        pos(pos),
        parent(parent),
        g(g),
        h(h) {}
    // Total cost at this node
    // f(n) = g(n) + h(n)
    int cost() const {
        return g + h;
    }
    xd::ivec2 tile_pos() const {
        return xd::ivec2(
            static_cast<int>(pos.x) / tile_width,
            static_cast<int>(pos.y) / tile_height);
    }
    bool operator==(const Node& other) const {
        return other.tile_pos() == tile_pos();
    }
    bool operator<(const Node& other) const {
        return cost() > other.cost();
    }
    // Check if another node is within the range of this one
    bool in_range(Node& other, int range) const;
};
template <typename T>
class Heap {
public:
    void push(T element) {
        elements.push_back(element);
        std::push_heap(elements.begin(), elements.end());
    }
    T pop() {
        std::pop_heap(elements.begin(), elements.end());
        T last = elements.back();
        elements.pop_back();
        return last;
    }
    int index(T element) {
        auto iter = std::find(elements.begin(), elements.end(), element);
        if (iter != elements.end())
            return iter - elements.begin();
        else
            return -1;
    }
    bool empty() const {
        return elements.empty();
    }
    T& operator[](int index) {
        return elements[index];
    }
    void update() {
        std::make_heap(elements.begin(), elements.end());
    }
private:
    std::vector<T> elements;
};

class Map;
class Map_Object;

class Pathfinder {
public:
    Pathfinder(Map& map, Map_Object& object,
        xd::vec2 dest, int range = 0, bool close = false,
        Collision_Check_Types check_type = Collision_Check_Types::BOTH);
    // The actual pathfinding step, call each frame
    void calculate_path();
    // Generate final path
    std::deque<Direction> generate_path();
    // Check if path was found
    bool is_found() const { return found; }
    // Get/set nearest node
    Node& nearest() { return nearest_node; }
private:
    // Add a node to an array of nodes
    void add_node(std::vector<Node>& nodes, xd::vec2 pos, Node& parent);
    // Distance between two points (heuristic)
    int distance(xd::ivec2 pos1, xd::ivec2 pos2) const {
        int dx = std::abs(pos1.x - pos2.x);
        int dy = std::abs(pos1.y - pos2.y);
        return std::max(dx, dy) ;
    }
    // Get a list of adjacent nodes to the given node
    std::vector<Node> get_adjacent_nodes(Node& node);
    // Can we skip this node? (because it's already in a list)
    bool skip_node(const Node& node);
    // No. of frames to wait before attempting to find another path in case
    // of collision; -1 disables such collision behevior. 
    const static int collision_wait = 5;
    // Maximum no. of attempts to find another path on collision. (-1 = infinity)
    const static int collision_limit = 30;
    // Game map
    Map& map;
    // The object to calculate a path for
    Map_Object& object;
    // The goal!
    Node goal_node;
    // The starting point
    Node start_node;
    // Was the path found?
    bool found;
    // Size of goal area
    int range;
    // If true and no path is found then get as close as possible
    bool get_close;
    // Goal node before pathfinding
    Node original_goal;
    // Number of times path was obstructed and recalculated
    int collision_counter;
    // List of nodes to be checked
    Heap<Node> open_list;
    // Hash table of already checked tiles
    std::unordered_map<xd::ivec2, Node> closed_list;
    // Nearest node found
    Node nearest_node;
    // Collision checking type
    Collision_Check_Types check_type;
};


#endif
