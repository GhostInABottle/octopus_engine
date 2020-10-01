#ifndef HPP_SHOW_TEXT_COMMAND
#define HPP_SHOW_TEXT_COMMAND

#include <vector>
#include <memory>
#include "../xd/graphics/types.hpp"
#include "../command.hpp"
#include "text_options.hpp"

class Game;

class Show_Text_Command : public Command {
public:
    Show_Text_Command(Game& game, Text_Options options);
    ~Show_Text_Command();
    void execute() override;
    void execute(int ticks) override;
    bool is_complete() const override;
    int choice_index();
    void set_start_time(long start);
private:
    struct Impl;
    friend struct Impl;
    std::unique_ptr<Impl> pimpl;
};

#endif
