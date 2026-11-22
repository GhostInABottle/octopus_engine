#ifndef HPP_COMMAND
#define HPP_COMMAND

#include <cstddef>
#include <optional>

class Map;

class Command {
public:
    Command() noexcept : stopped(false), force_stopped(false),
        paused(false), map_sequence_number(std::nullopt) {}
    // Get the sequence number of the map associated with this command, if any
    std::optional<std::size_t> get_map_sequence_number() const {
        return map_sequence_number;
    }
    // Called while the command isn't completed
    virtual void execute() = 0;
    // Version with specific time
    virtual void execute(int) { execute(); }
    // Is the command completed?
    virtual bool is_complete() const = 0;
    // Would the command be complete by the given time?
    virtual bool is_complete(int) const { return is_complete(); }
    // Ask the command to complete ASAP
    virtual void stop() { stopped = true; }
    virtual bool is_stopped() const { return stopped; }
    // Stop the command and immediately mark it as complete
    virtual void force_stop() {
        stopped = true;
        force_stopped = true;
    }
    // Pause the command to temporarily stop executing it
    virtual void pause() { paused = true; }
    // Pause the command at specific time
    virtual void pause(int) { pause(); }
    // Resume a paused command
    virtual void resume() { paused = false; }
    // Is the command paused
    virtual bool is_paused() const { return paused;  }
    // Virtual destructor
    virtual ~Command() = 0;
protected:
    bool stopped;
    bool force_stopped;
    bool paused;
    std::optional<std::size_t> map_sequence_number;
};

#endif
