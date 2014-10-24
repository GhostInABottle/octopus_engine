#ifndef HPP_NPC
#define HPP_NPC

#include <string>
#include <memory>
#include <xd/graphics/types.hpp>
#include "rapidxml.hpp"

class Map_Object;
class Game;
struct Keypoint;

// Non-playable characters with schedules
class NPC {
public:
    // Constructor and destructor
    NPC(Game& game);
    ~NPC();
    // Update NPC, called each frame
    void update();
    // Get current keypoint's day
    int keypoint_day() const;
    // Get current keypoint's day type (even, odd, all)
    std::string keypoint_day_type() const;
    // Get current keypoint's starting time
    int keypoint_time() const;
    // Check if NPC has a schedule with the given name
    bool has_schedule(const std::string& schedule_name) const;
    // Set current schedule
    void set_schedule(const std::string& schedule_name);
    // Get current schedule name
    std::string get_schedule() const;
    // Get a keypoint with given schedule name and index
    Keypoint* get_keypoint(const std::string& schedule_name, unsigned int index);
    // Load an NPC from an XML file
    static std::unique_ptr<NPC> load(Game& game, const std::string& filename);
    // Load map from a XML node
    static std::unique_ptr<NPC> load(Game& game, rapidxml::xml_node<>& node);
    // Getters and setters
    std::string get_name() const {
        return name;
    }
    std::string get_display_name() const {
        return display_name;
    }
    std::string get_map_name() const {
        return map;
    }
    xd::vec2 get_position() const {
        return position;
    }
    Map_Object* get_object() {
        return object;
    }
    bool is_active() const {
        return active;
    }
    void set_active(bool active) {
        this->active = active;
    }
private:
    // Unique NPC name
    std::string name;
    // NPC display name
    std::string display_name;
    // Current map
    std::string map;
    // Last position
    xd::vec2 position;
    // Object on current map (if any)
    Map_Object* object;
    // Is NPC's schedule active?
    bool active;
    // Implementation details
    struct Impl;
    friend struct Impl;
    std::unique_ptr<Impl> pimpl;
};

#endif
