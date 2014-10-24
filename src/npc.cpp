#include "../include/npc.hpp"
#include "../include/keypoint.hpp"
#include "../include/common.hpp"
#include "../include/utility.hpp"
#include "../include/direction_utilities.hpp"
#include "../include/exceptions.hpp"
#include "../include/game.hpp"
#include "../include/clock.hpp"
#include "../include/map.hpp"
#include "../include/map_object.hpp"
#include "../include/commands.hpp"
#include "../include/configurations.hpp"
#include "../include/log.hpp"
#include <boost/lexical_cast.hpp>
#include <cmath>
#include <algorithm>

struct NPC::Impl {
    Impl(Game& game, NPC& npc) :
        game(game), 
        npc(npc),
        last_keypoint(nullptr),
        expected_completion(-1),
        moving_to_keypoint(false),
        visible(true),
        passthrough(false),
        direction(Direction::NONE)
    {
        if (time_multiplier < 0) {
            time_multiplier = static_cast<int>(
                Configurations::get<float>("game.time-multiplier"));
            frame_time = 1000 / Configurations::get<int>("game.logic-fps");
        }
    }
    // Game instance
    Game& game;
    // NPC owner
    NPC& npc;
    // Sprite file name
    std::string sprite;
    // NPC attributes
    Properties properties;
    // Last executed keypoint
    Keypoint* last_keypoint;
    // Map to which position is tied
    std::string position_map;
    // Currently active schedule
    std::string current_schedule;
    // Schedules, each is a list of keypoints
    std::unordered_map<std::string, std::vector<Keypoint>> schedules;
    // Currently executing script command
    std::unique_ptr<Simulatable_Command> script_command;
    // Estimated completion time of keypoint on other map
    int expected_completion;
    // Estimated position of keypoint on other map
    xd::vec2 expected_position;
    // Is the object moving to the keypoint position
    bool moving_to_keypoint;
    // Multiplier of game time speed
    static int time_multiplier;
    // Time taken for a single frame
    static int frame_time;
    // Is NPC visible?
    bool visible;
    // Can walk through the NPC?
    bool passthrough;
    // NPC direction
    Direction direction;
    // Set current schedule
    void set_schedule(const std::string& schedule_name) {
        if (!npc.has_schedule(schedule_name)) {
            LOGGER_W << "Invalid schedule " << schedule_name << " for NPC " << npc.name;
            return;
        }
        // Reset keypoints and current schedule state
        for (auto& keypoint : schedules[current_schedule]) {
            keypoint.reset();
        }
        last_keypoint = nullptr;
        expected_completion = -1;
        moving_to_keypoint = false;
        script_command.reset(nullptr);
        current_schedule = schedule_name;
    }
    // Set current keypoint
    void set_keypoint(Keypoint* keypoint, bool same_map) {
        last_keypoint = keypoint;
        if (npc.object && same_map) {
            // Set object's properties to keypoint's
            if (keypoint->direction != Direction::NONE) {
                bool has_face_command = std::any_of(
                    keypoint->commands.begin(), keypoint->commands.end(),
                    [](const Keypoint::Command& cmd) {
                        return cmd.type == Keypoint::Command::Types::FACE;
                    }
                );
                if (!has_face_command) {
                    direction = keypoint->direction;
                    npc.object->face(direction);
                }
            }
            npc.object->set_script(keypoint->activation_script);
            if (npc.object->get_pose_name() != keypoint->pose) {
                npc.object->set_pose(keypoint->pose);
            }
        }
    }
    // Process the next keypoint command
    void process_command() {
        if (!npc.object)
            return;
        script_command.reset(nullptr);
        using boost::any_cast;
        int commands_size = static_cast<int>(last_keypoint->commands.size());
        // Check if all commands are processed (keypoint completed)
        if (last_keypoint->command_index >= commands_size) {
            complete_keypoint();
            return;
        }
        // Process command by type
        auto command = last_keypoint->commands[last_keypoint->command_index];
        switch(command.type) {
        case Keypoint::Command::Types::MOVE:
            script_command.reset(
                new Move_Object_To_Command(
                    *game.get_map(),
                    *npc.object,
                    any_cast<float>(command.args["x"]),
                    any_cast<float>(command.args["y"]),
                    Collision_Check_Types::TILE,
                    true
                )
            );
            break;
        case Keypoint::Command::Types::FACE:
            direction = any_cast<Direction>(command.args["dir"]);
            npc.object->face(direction);
            break;
        case Keypoint::Command::Types::TELEPORT:
            npc.map = any_cast<std::string>(command.args["map"]);
            position_map = npc.map;
            npc.position = xd::vec2(
                any_cast<float>(command.args["x"]),
                any_cast<float>(command.args["y"])
            );
            delete_object();
            complete_keypoint();
            break;
        case Keypoint::Command::Types::WAIT:
            script_command.reset(
                new Wait_Command(game,
                    any_cast<int>(command.args["duration"]) * 1000,
                    time_without_days(game.get_clock()->total_seconds()) * 1000
                )
            );
            break;
        case Keypoint::Command::Types::VISIBILITY:
            visible = any_cast<bool>(command.args["value"]);
            npc.object->set_visible(visible);
            break;
        case Keypoint::Command::Types::PASSTHROUGH:
            passthrough = any_cast<bool>(command.args["value"]);
            npc.object->set_passthrough(passthrough);
            break;
        }
        if (!script_command)
            script_command.reset(new Dummy_Command);
    }
    // Process keypoints on other maps
    void process_offmap_command(int time) {
        using boost::any_cast;
        int commands_size = static_cast<int>(last_keypoint->commands.size());
        // Check if all commands are processed (keypoint completed)
        if (last_keypoint->command_index >= commands_size) {
            complete_keypoint();
            position_map = npc.map;
            npc.position = expected_position;
            return;
        }
        expected_position = npc.position;
        expected_completion = time;
        // Calculate expected time and position for commands
        float x, y;
        auto command = last_keypoint->commands[last_keypoint->command_index];
        switch(command.type) {
        case Keypoint::Command::Types::MOVE:
            x = any_cast<float>(command.args["x"]);
            y = any_cast<float>(command.args["y"]);
            process_offmap_move(xd::vec2(x, y), time);
            break;
        case Keypoint::Command::Types::FACE:
            direction = any_cast<Direction>(command.args["dir"]);
            break;
        case Keypoint::Command::Types::TELEPORT:
            npc.map = any_cast<std::string>(command.args["map"]);
            expected_position = xd::vec2(
                any_cast<float>(command.args["x"]),
                any_cast<float>(command.args["y"])
            );
            position_map = npc.map;
            npc.position = expected_position;
            complete_keypoint();
            break;
        case Keypoint::Command::Types::WAIT:
            expected_completion = time +
                any_cast<int>(command.args["duration"]);
            break;
        case Keypoint::Command::Types::VISIBILITY:
            visible = any_cast<bool>(command.args["value"]);
            break;
        case Keypoint::Command::Types::PASSTHROUGH:
            passthrough = any_cast<bool>(command.args["value"]);
            break;
        default:
            break;
        }
    }
    // Process a move command on another map
    void process_offmap_move(const xd::vec2& dest, int time) {
        expected_position = dest;
        float distance =  std::max(
            std::abs(dest.x - npc.position.x),
            std::abs(dest.y - npc.position.y)) * 1.25f;
        static float frame_time = static_cast<float>(1000 /
            Configurations::get<int>("game.logic-fps"));
        static float multiplier = Configurations::get<float>("game.time-multiplier");
        int delay = static_cast<int>(multiplier * distance * frame_time / 1000.0f);
        expected_completion = time + delay;
    }
    // Execute a pending command, return true while processing
    bool execute_pending_command(int time) {
        if (!script_command)
            return false;
        if (script_command->is_complete(time * 1000)) {
            if (last_keypoint->status != Keypoint::Status_Types::PENDING) {
                // Normal KP command processing
                last_keypoint->command_index++;
                process_command();
            } else {
                // NPC was moving to KP start pos
                script_command.reset(nullptr);
            }
        } else {
            script_command->execute();
        }
        if (npc.object)
            npc.position = npc.object->get_real_position();
        return true;
    }
    // Find the best keypoint matching current time
    std::pair<int, Keypoint*> find_best_keypoint(int day, int time) {
        auto& schedule = schedules[current_schedule];
        unsigned int best_index = 0;
        Keypoint* best_keypoint = nullptr;
        for (unsigned int i = 0; i < schedule.size(); ++i) {
            auto& keypoint = schedule[i];
            bool day_match = day % keypoint.day == 0;
            // Check for odd and even days
            if (keypoint.day_type == Keypoint::Day_Types::EVEN && day & 1)
                day_match = false;
            if (keypoint.day_type == Keypoint::Day_Types::ODD && !(day & 1))
                day_match = false;
            if (keypoint.day != -1 && day_match && keypoint.timestamp <= time) {
                if (!best_keypoint || keypoint.timestamp > best_keypoint->timestamp) {
                    best_keypoint = &keypoint;
                    best_index = i;
                }
            }
        }
        return std::make_pair(best_index, best_keypoint);
    }
    // Delete the NPC's map object
    void delete_object(bool delete_from_map = true) {
        if (delete_from_map) {
            game.get_map()->delete_object(npc.object);
        }
        npc.object = nullptr;
        script_command.reset(nullptr);
    }
    // Mark current keypoint as completed
    void complete_keypoint() {
        last_keypoint->status = Keypoint::Status_Types::COMPLETED;
        int seconds = game.get_clock()->total_seconds();
        last_keypoint->completion_day = time_to_days(seconds);
        last_keypoint->command_index = 0;
        expected_completion = -1;
    }
    // Get the most recent (incomplete) sequential keypoint
    Keypoint* advance_keypoint(Keypoint* keypoint, unsigned int index, int day) {
        auto& schedule = schedules[current_schedule];
        auto prior_day = keypoint->day;
        auto prior_day_type = keypoint->day_type;
        while (keypoint->status == Keypoint::Status_Types::COMPLETED) {
            if (keypoint->completion_day != day)
                keypoint->status = Keypoint::Status_Types::PENDING;
            else if (keypoint->sequential && index + 1 < schedule.size()) {
                keypoint = &schedule[++index];
                keypoint->day = prior_day;
                keypoint->day_type = prior_day_type;
            } else
                break;
        }
        return keypoint;
    }
    // If NPC isn't at keypoint's position move NPC there
    bool move_to_keypoint(int time) {
        auto player = game.get_player();
        bool old_player_passthrough = player->is_passthrough();
        if (last_keypoint->status == Keypoint::Status_Types::PENDING) {
            // Set start time and timestamp if not already set
            if (last_keypoint->start_time < 0)
                last_keypoint->start_time = time;
            if (last_keypoint->timestamp < 0)
                last_keypoint->timestamp = time;
            auto obj_pos = npc.object->get_real_position();
            auto& last_pos = last_keypoint->position;
            auto dx = std::abs(obj_pos.x - last_pos.x);
            auto dy = std::abs(obj_pos.y - last_pos.y);
            if (dx >= 8.0f || dy >= 8.0f) {
                script_command.reset(
                    new Move_Object_To_Command(
                        *game.get_map(),
                        *npc.object,
                        last_pos.x,
                        last_pos.y,
                        Collision_Check_Types::TILE,
                        true
                    )
                );
                int simulated_time = 0;
                int time_passed = (time - last_keypoint->timestamp) / time_multiplier;
                // Simulate movement to keypoint position since timestamp
                if (time_passed > 1) {
                    // Make player passable to ensure object isn't stuck at player pos
                    player->set_passthrough(true);
                    while (simulated_time <= time_passed * 1000) {
                        if (script_command->is_complete(time * 1000 + simulated_time)) {
                            script_command.reset(nullptr);
                            break;
                        } else {
                            script_command->execute();
                        }
                        position_map = npc.map;
                        npc.position = npc.object->get_real_position();
                        simulated_time += frame_time;
                    }
                    player->set_passthrough(old_player_passthrough);
                }
                return true;
            }
            position_map = npc.map;
            npc.position = last_keypoint->position;
            last_keypoint->status = Keypoint::Status_Types::STARTED;
            last_keypoint->start_time = time;
            npc.object->set_script(last_keypoint->activation_script);
        }
        return false;
    }
    // If NPC isn't at key point's position move it there(on another map)
    bool move_to_offmap_keypoint(int time) {
        if (last_keypoint->timestamp < 0)
            last_keypoint->timestamp = time;
        auto dx = std::abs(npc.position.x - last_keypoint->position.x);
        auto dy = std::abs(npc.position.y - last_keypoint->position.y);
        if (dx >= 8.0f || dy >= 8.0f) {
            process_offmap_move(last_keypoint->position, time);
            moving_to_keypoint = true;
            return true;
        }
        expected_completion = -1;
        expected_position = npc.position;
        last_keypoint->status = Keypoint::Status_Types::STARTED;
        last_keypoint->start_time = time;
        return false;
    }
    // Simulate on-map commands that happened since keypoint started
    void simulate_commands(int time, int time_passed) {
        auto player = game.get_player();
        // Make player passable to ensure object isn't stuck at player pos
        bool old_player_passthrough = player->is_passthrough();
        player->set_passthrough(true);
        last_keypoint->status = Keypoint::Status_Types::STARTED;
        int simulated_time = 0;
        process_command();
        while (script_command && simulated_time <= time_passed * 1000) {
            if (script_command->is_complete(time * 1000 + simulated_time)) {
                last_keypoint->command_index++;
                process_command();
            } else {
                script_command->execute();
            }
            if (npc.object) {
                position_map = npc.map;
                npc.position = npc.object->get_real_position();
            }
            simulated_time += frame_time;
        }
        player->set_passthrough(old_player_passthrough);
    }
    std::string get_map_filename() {
        return game.get_map()->get_filename();
    }
    // Parse a formatted time string
    static int parse_time(const std::string& time_val) {
        using boost::lexical_cast;
        // Time can be given as a seconds timestamp or formatted h:m:s
        if (time_val.find_first_of(':') == std::string::npos)
            return lexical_cast<int>(time_val);
        else {
            auto components = split(time_val, ":");
            int hour, minute, second;
            minute = second = 0;
            hour = lexical_cast<int>(components[0]);
            if (components.size() > 1) {
                minute = lexical_cast<int>(components[1]);
                if (components.size() > 2)
                    second = lexical_cast<int>(components[2]);
            }
            if (hour > 11 || minute > 59 || second > 59)
                throw xml_exception("Invalid NPC file. Invalid time format");
            return hour * 3600 + minute * 60 + second;
        }
    }
};

int NPC::Impl::time_multiplier = -1;
int NPC::Impl::frame_time = -1;

NPC::NPC(Game& game) :
    pimpl(new Impl(game, *this)), position(-1.0f, -1.0f),
    active(true), object(nullptr) {}

NPC::~NPC() {}

void NPC::update() {
    bool same_map = map == pimpl->get_map_filename();
    if (!same_map) {
        // Reset object and script command if player left the map
        pimpl->delete_object(false);
    }
    if (!active) {
        if (same_map && object)
            object->update_state("FACE");
        return;
    }
    auto clock = pimpl->game.get_clock();
    int day = time_to_days(clock->total_seconds());
    int time = time_without_days(clock->total_seconds());
    if (same_map && pimpl->execute_pending_command(time))
        return;
    auto best_pair = pimpl->find_best_keypoint(day, time);
    Keypoint* best_keypoint = best_pair.second;
    if (!best_keypoint) {
        pimpl->delete_object();
        return;
    }
    best_keypoint = pimpl->advance_keypoint(best_keypoint, best_pair.first, day);
    // Set NPC position to keypoint map and position
    map = best_keypoint->map;
    same_map = map == pimpl->get_map_filename();
    if (position.x > -1.0f && pimpl->position_map != map)
        position = xd::vec2(-1.0f, -1.0f);
    if (position.x < 0.0f) {
        pimpl->position_map = map;
        position = best_keypoint->position;
    }
    // If previous keypoint was on this map
    if (same_map) {
        pimpl->expected_completion = -1;
        // If NPC isn't on the map create object at keypoint position
        if (!object) {
            auto map_ptr = pimpl->game.get_map();
            if (auto obj = map_ptr->get_object(name))
                object = obj;
            else
                object = map_ptr->create_object(name, pimpl->sprite, position);
            object->set_type("npc");
            if (pimpl->direction != Direction::NONE)
                object->set_direction(pimpl->direction);
        }
        object->set_visible(pimpl->visible);
        object->set_passthrough(pimpl->passthrough);
        pimpl->set_keypoint(best_keypoint, true);
        if (pimpl->move_to_keypoint(time))
            return;
        if (pimpl->last_keypoint->status == Keypoint::Status_Types::COMPLETED)
            return;
        int time_passed =
            (time - pimpl->last_keypoint->start_time) / Impl::time_multiplier;
        if (time_passed > 1) {
            pimpl->simulate_commands(time, time_passed);
        } else {
            pimpl->process_command();
        }
    } else { // If action is on another map
        // Wait for any pending commands
        if (time < pimpl->expected_completion) {
            return;
        } else if (pimpl->expected_completion > 0) {
            if (pimpl->last_keypoint->status == Keypoint::Status_Types::STARTED) {
                // Previous command completed, process next command
                position = pimpl->expected_position;
                pimpl->last_keypoint->command_index++;
            } else if (pimpl->moving_to_keypoint) {
                // Reached keypoint position, can start now
                pimpl->position_map = map;
                position = pimpl->last_keypoint->position;
                pimpl->moving_to_keypoint = false;
            }
        }
        pimpl->set_keypoint(best_keypoint, false);
        if (pimpl->last_keypoint->status == Keypoint::Status_Types::PENDING) {
            if (pimpl->move_to_offmap_keypoint(time))
                return;
        } else if (pimpl->last_keypoint->status == Keypoint::Status_Types::COMPLETED) {
            return;
        }
        pimpl->process_offmap_command(time);
    }
}

int NPC::keypoint_day() const {
    return pimpl->last_keypoint ? pimpl->last_keypoint->day : -1;
}

std::string NPC::keypoint_day_type() const {
    std::string type = "all";
    if (pimpl->last_keypoint) {
        switch(pimpl->last_keypoint->day_type) {
        case Keypoint::Day_Types::EVEN:
            type = "even";
            break;
        case Keypoint::Day_Types::ODD:
            type = "odd";
            break;
        }
    }   
    return type;
}

int NPC::keypoint_time() const {
    return pimpl->last_keypoint ? pimpl->last_keypoint->start_time : -1;
}

bool NPC::has_schedule(const std::string& schedule_name) const {
    return pimpl->schedules.find(schedule_name) != pimpl->schedules.end();
}

void NPC::set_schedule(const std::string& schedule_name) {
    pimpl->set_schedule(schedule_name);
}

std::string NPC::get_schedule() const {
    return pimpl->current_schedule;
}

Keypoint* NPC::get_keypoint(const std::string& schedule_name, unsigned int index) {
    if (pimpl->schedules.find(schedule_name) != pimpl->schedules.end()) {
        auto& schedule = pimpl->schedules[schedule_name];
        if (index < schedule.size())
            return &schedule[index];
    }
    return nullptr;
}

std::unique_ptr<NPC> NPC::load(Game& game, const std::string& filename) {
    // Open the file and load the NPC node
    rapidxml::memory_pool<> pool;
    char* content = pool.allocate_string(read_file(filename).c_str());
    rapidxml::xml_document<> doc;
    doc.parse<0>(content);
    auto node = doc.first_node("npc");
    if (!node)
        throw xml_exception("Invalid NPC file. Missing npc node");
    return load(game, *node);
}

std::unique_ptr<NPC> NPC::load(Game& game, rapidxml::xml_node<>& node) {
    using boost::lexical_cast;
    std::unique_ptr<NPC> npc_ptr(new NPC(game));

    npc_ptr->name = node.first_attribute("name")->value();
    if (auto display_name_attr = node.first_attribute("display"))
        npc_ptr->display_name = display_name_attr->value();
    else
        npc_ptr->display_name = npc_ptr->name;
    npc_ptr->pimpl->sprite = node.first_attribute("sprite")->value();
    read_properties(npc_ptr->pimpl->properties, node);

    for (auto schedule_node = node.first_node("schedule");
            schedule_node; schedule_node = schedule_node->next_sibling("schedule")) {    
        // Get schedule's name if specified
        std::string schedule_name = "default";
        if (schedule_node->first_attribute("name"))
            schedule_name = schedule_node->first_attribute("name")->value();
        // Get default values for activation script and day type
        std::string schedule_script, schedule_day;
        if (auto attr = schedule_node->first_attribute("activation"))
            schedule_script = attr->value();
        if (auto attr = schedule_node->first_attribute("day"))
            schedule_day = capitalize(attr->value());
        // Load the keypoints
        std::vector <Keypoint> keypoints;
        for (auto keypoint_node = schedule_node->first_node("keypoint");
                keypoint_node; keypoint_node = keypoint_node->next_sibling("keypoint")) {
            Keypoint keypoint;
            keypoint.map = keypoint_node->first_attribute("map")->value();
            keypoint.position.x = lexical_cast<float>(
                keypoint_node->first_attribute("x")->value());
            keypoint.position.y = lexical_cast<float>(
                keypoint_node->first_attribute("y")->value());
            keypoint.sequential = false;
            keypoint.day = -1;
            keypoint.timestamp = -1;
            if (auto attr = keypoint_node->first_attribute("sequential")) {
                std::string value(attr->value());
                keypoint.sequential = capitalize(value) == "TRUE";
            }
            // Get pose, direction and scripts, if any
            if (auto attr = keypoint_node->first_attribute("pose"))
                keypoint.pose = attr->value();
            if (auto attr = keypoint_node->first_attribute("direction"))
                keypoint.direction = string_to_direction(attr->value());
            if (auto attr = keypoint_node->first_attribute("activation"))
                keypoint.activation_script = attr->value();
            else if (!schedule_script.empty())
                keypoint.activation_script = schedule_script;
            if (auto attr = keypoint_node->first_attribute("reach"))
                keypoint.reach_script = attr->value();
            // Load timestamp
            if (auto time_node = keypoint_node->first_node("time")) {
                std::string day_val;
                if (auto attr = time_node->first_attribute("day"))
                    day_val = capitalize(attr->value());
                else
                    day_val = capitalize(schedule_day);
                if (!day_val.empty()) {
                    if (day_val == "EVEN" || day_val == "ODD") {
                        // Repeat every other even or odd day
                        keypoint.day_type = day_val == "EVEN" ?
                            Keypoint::Day_Types::EVEN : Keypoint::Day_Types::ODD;
                        keypoint.day = 1;
                    } else {
                        // Repeat every n days
                        keypoint.day = lexical_cast<int>(day_val);
                    }
                } else {
                    // Repeat daily
                    keypoint.day = 1;
                }
                if (auto attr = time_node->first_attribute("timestamp")) {
                    std::string time_val = attr->value();
                    keypoint.timestamp = Impl::parse_time(time_val);
                } else {
                    throw xml_exception("Invalid NPC file. Missing time attribute");
                }
            } else if (!keypoints.empty() && !keypoints.back().sequential) {
                throw xml_exception("Invalid NPC file. Keypoint must have a time");
            }
            // Load commands
            if (auto commands_node = keypoint_node->first_node("commands")) {
                for (auto cmd_node = commands_node->first_node("command");
                        cmd_node;
                        cmd_node = cmd_node->next_sibling("command")) {
                    Keypoint::Command command;
                    // Get command type
                    std::string type_str = capitalize(
                        cmd_node->first_attribute("type")->value());
                    static std::unordered_map<std::string, Keypoint::Command::Types> types_map;
                    if (types_map.empty()) {
                        types_map["MOVE"] = Keypoint::Command::Types::MOVE;
                        types_map["FACE"] = Keypoint::Command::Types::FACE;
                        types_map["TELEPORT"] = Keypoint::Command::Types::TELEPORT;
                        types_map["WAIT"] = Keypoint::Command::Types::WAIT;
                        types_map["VISIBILITY"] = Keypoint::Command::Types::VISIBILITY;
                        types_map["PASSTHROUGH"] = Keypoint::Command::Types::PASSTHROUGH;
                    }
                    if (types_map.find(type_str) != types_map.end())
                        command.type = types_map[type_str];
                    else
                        throw xml_exception("Invalid NPC file. Invalid command type");
                    // Get command arguments
                    std::string value;
                    switch(command.type) {
                    case Keypoint::Command::Types::MOVE:
                        command.args["x"] = lexical_cast<float>(
                            cmd_node->first_attribute("x")->value());
                        command.args["y"] = lexical_cast<float>(
                            cmd_node->first_attribute("y")->value());
                        break;
                    case Keypoint::Command::Types::FACE:
                        command.args["dir"] = string_to_direction(
                            cmd_node->first_attribute("dir")->value());
                        break;
                    case Keypoint::Command::Types::TELEPORT:
                        command.args["x"] = lexical_cast<float>(
                            cmd_node->first_attribute("x")->value());
                        command.args["y"] = lexical_cast<float>(
                            cmd_node->first_attribute("y")->value());
                        command.args["map"] = std::string(
                            cmd_node->first_attribute("map")->value());
                        break;
                    case Keypoint::Command::Types::WAIT:
                        value = cmd_node->first_attribute("duration")->value();
                        command.args["duration"] = Impl::parse_time(value);
                        break;
                    case Keypoint::Command::Types::VISIBILITY:
                    case Keypoint::Command::Types::PASSTHROUGH:
                        value = cmd_node->first_attribute("value")->value();
                        command.args["value"] = capitalize(value) == "TRUE";
                        break;
                    }
                    keypoint.commands.push_back(command);
                }
            }
            keypoints.push_back(keypoint);
        }
        auto& schedule = npc_ptr->pimpl->schedules;
        schedule[schedule_name] = keypoints;
        npc_ptr->pimpl->last_keypoint = nullptr;
        if (!keypoints.empty())
            npc_ptr->pimpl->last_keypoint = &schedule[schedule_name][0];
    }

    if (npc_ptr->pimpl->schedules.empty())
        throw xml_exception("Invalid NPC file. Must have at least one schedule");
    npc_ptr->pimpl->current_schedule = npc_ptr->pimpl->schedules.begin()->first;

    return npc_ptr;
}
