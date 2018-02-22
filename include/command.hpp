#ifndef HPP_COMMAND
#define HPP_COMMAND

class Command {
public:
    Command() : stopped(false) {}
    // Called while the command isn't completed
    virtual void execute() = 0;
    // Is the command completed?
    virtual bool is_complete() const = 0;
    // Would the command be complete by the given time?
    virtual bool is_complete(int ticks) const { return is_complete(); }
    // Force the command to complete
    virtual void stop() { stopped = true; }
    virtual ~Command() = 0;
protected:
    bool stopped;
};

#endif
