#ifndef HPP_SHOW_TEXT_COMMAND
#define HPP_SHOW_TEXT_COMMAND

#include "../commands/command.hpp"
#include "text_options.hpp"
#include <memory>

class Game;

class Show_Text_Command : public Command {
public:
    Show_Text_Command(Game& game, Text_Options options);
    ~Show_Text_Command();
    void execute() override;
    void execute(int ticks) override;
    bool is_complete() const override;
    void pause() noexcept override;
    void pause(int ticks = -1) noexcept override;
    void resume() noexcept override;
    int choice_index();
    void set_start_time(int start);
private:
    struct Impl;
    friend struct Impl;
    std::unique_ptr<Impl> pimpl;
};

#endif
