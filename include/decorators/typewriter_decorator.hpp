#ifndef HPP_TYPEWRITER_DECORATOR
#define HPP_TYPEWRITER_DECORATOR

#include <string>
#include <unordered_map>

class Game;
namespace xd {
    class text_decorator;
    class formatted_text;
    class text_decorator_args;
}

class Typewriter_Decorator {
public:
    Typewriter_Decorator(Game& game);
    void operator()(xd::text_decorator& decorator, const xd::formatted_text& text, const xd::text_decorator_args& args);
    bool is_done(Game& game, int slot) const {
        if (states.find(slot) == states.end()) return true;
        return states.at(slot).done;
    }
    void reset() {
        states.clear();
    }
private:
    Game& game;
    struct State {
        long start_time{0};
        bool game_paused{false};
        bool done{false};
        std::string text;
        int ticks(Game& game) const;
    };
    std::unordered_map<int, State> states;
};

#endif