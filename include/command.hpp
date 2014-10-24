#ifndef HPP_COMMAND
#define HPP_COMMAND

class Command {
public:
    virtual void execute() = 0;
    virtual bool is_complete() const = 0;
    bool operator()() const { return is_complete(); }
    virtual ~Command() = 0;
};

class Simulatable_Command : public Command {
public:
    using Command::is_complete;
    virtual bool is_complete(int ticks) const = 0;
};

#endif
