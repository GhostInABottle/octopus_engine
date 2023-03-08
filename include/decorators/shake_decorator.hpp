#ifndef HPP_SHAKE_DECORATOR
#define HPP_SHAKE_DECORATOR

#include <string>
#include <unordered_map>
#include <vector>
#include "../xd/glm.hpp"

class Game;
namespace xd {
    class text_decorator;
    class formatted_text;
    class text_decorator_args;
}

class Shake_Decorator {
public:
    Shake_Decorator(Game& game);
    void operator()(xd::text_decorator& decorator, const xd::formatted_text& text, const xd::text_decorator_args& args);
private:
    Game& game;
    struct State {
        long last_shake{0};
        bool shake{false};
        bool game_paused{false};
        std::vector<xd::ivec2> displacements;
        int ticks(Game& game) const;
    };
    std::unordered_map<std::string, State> states;
};

#endif