#ifndef HPP_COMMAND
#define HPP_COMMAND

class Map;

class Command {
public:
    Command() noexcept : stopped(false), paused(false), map_ptr(nullptr) {}
    // Called while the command isn't completed
    virtual void execute() = 0;
    // Version with specific time
    virtual void execute(int) { execute(); }
    // Is the command completed?
    virtual bool is_complete() const = 0;
    // Would the command be complete by the given time?
    virtual bool is_complete(int) const { return is_complete(); }
    // Force the command to complete
    virtual void stop() noexcept { stopped = true; }
    virtual bool is_stopped() const noexcept { return stopped; }
    // Pause the command to temporarily stop executing it
    virtual void pause() noexcept { paused = true; }
    // Pause the command at specific time
    virtual void pause(int) noexcept { pause(); }
    // Resume a paused command
    virtual void resume() noexcept { paused = false; }
    // Is the command paused
    virtual bool is_paused() const noexcept { return paused;  }
    // Get the map associated with the command, if any
    const Map* get_map_ptr() const { return map_ptr; }
    // Virtual destructor
    virtual ~Command() = 0;
protected:
    bool stopped;
    bool paused;
    Map* map_ptr;
};

#endif
