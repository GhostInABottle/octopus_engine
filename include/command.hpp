#ifndef HPP_COMMAND
#define HPP_COMMAND

class Command {
public:
    Command() : stopped(false), paused(false) {}
    // Called while the command isn't completed
    virtual void execute() = 0;
    // Version with specific time
    virtual void execute(int) { execute(); }
    // Is the command completed?
    virtual bool is_complete() const = 0;
    // Would the command be complete by the given time?
    virtual bool is_complete(int) const { return is_complete(); }
    // Force the command to complete
    virtual void stop() { stopped = true; }
    virtual bool is_stopped() const { return stopped; }
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
    bool paused;
};

#endif
