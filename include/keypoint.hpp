#ifndef HPP_KEYPOINT
#define HPP_KEYPOINT

#include <string>
#include <unordered_map>
#include <vector>
#include <boost/any.hpp>
#include <xd/graphics/types.hpp>
#include "direction.hpp"

// NPC schedule keypoint
struct Keypoint {
    // Map name
    std::string map;
    // Day (n = every n days)
    int day;
    // Time in seconds (0 to 43199 = 11 * 3600 + 59 * 60 + 59) 
    int timestamp;
    // Actual timestamp when keypoint was started
    int start_time;
    // Day type conditions (e.g. only on odd days)
    enum class Day_Types { ALL, EVEN, ODD } day_type;
    // Must the day be odd or even?
    // Keypoint position (NPC should be here at this key point)
    xd::vec2 position;
    // File name of script to execute during this keypoint
    std::string activation_script;
    // TODO: File name of script to execute once position is reached
    std::string reach_script;
    // Sprite pose during this keypoint
    std::string pose;
    // Facing direction
    Direction direction;
    // Advance to next keypoint after this one is done?
    bool sequential;
    // Keypoint action status types
    enum class Status_Types { PENDING, STARTED, COMPLETED };
    // Current action status
    Status_Types status;
    // Day on which point was last completed
    int completion_day;
    struct Command {
        enum class Types { MOVE, FACE, TELEPORT, WAIT, VISIBILITY, PASSTHROUGH };
        Types type;
        std::unordered_map<std::string, boost::any> args;
    };
    // List of commands to execute in this keypoint
    std::vector<Command> commands;
    // Current command index
    int command_index;
    // Constructor
    Keypoint() : start_time(-1), day_type(Day_Types::ALL),
        direction(Direction::NONE), status(Status_Types::PENDING),
        completion_day(0), command_index(0) {}
    // Reset keypoint
    void reset() {
        start_time = -1;
        status = Status_Types::PENDING;
        completion_day = 0;
        command_index = 0;
    }
    // Set the timestamp
    int set_time(int hour, int minute, int second) {
        return timestamp = hour * 3600 + minute * 60 + second;
    }
    // Get hour from timestamp
    int hour() const { return (timestamp / 3600) % 12; }
    // Get minutes from timestamp
    int minute() const { return (timestamp / 60) % 60; }
    // Get seconds from timestamp
    int second() const { return timestamp % 60; } 
};

#endif
