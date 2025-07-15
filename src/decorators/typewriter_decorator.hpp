#ifndef HPP_TYPEWRITER_DECORATOR
#define HPP_TYPEWRITER_DECORATOR

#include <string>
#include <unordered_map>

class Game;
class Audio_Player;
namespace xd {
    class text_decorator;
    class formatted_text;
    class text_decorator_args;
}

class Typewriter_Decorator {
public:
    Typewriter_Decorator(Game& game, Audio_Player& audio_player);
    void operator()(xd::text_decorator& decorator, const xd::formatted_text& text, const xd::text_decorator_args& args);
    bool is_done(int slot) const {
        if (states.find(slot) == states.end()) return false;
        return states.at(slot).done;
    }
    void reset_slot(int slot) {
        states.erase(slot);
    }
    void reset() {
        states.clear();
    }
private:
    Game& game;
    Audio_Player& audio_player;
    struct State {
        long start_time{0};
        bool game_paused{false};
        bool done{false};
        std::string text;
        int last_index{0};
        int ticks(Game& game) const;
    };
    std::unordered_map<int, State> states;
};

#endif