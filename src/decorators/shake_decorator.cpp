#include "../../include/configurations.hpp"
#include "../../include/decorators/shake_decorator.hpp"
#include "../../include/game.hpp"
#include "../../include/xd/graphics/text_formatter.hpp"
#include <random>

Shake_Decorator::Shake_Decorator(Game& game) : game(game) {}

void Shake_Decorator::operator()(xd::text_decorator& decorator, const xd::formatted_text& text, const xd::text_decorator_args& args) {

    // Delete old states
    for(auto it = std::begin(states); it != std::end(states);)
    {
      if (it->second.ticks(game) - it->second.last_shake > 1000)
        it = states.erase(it);
      else
        ++it;
    }

    auto power = 100 - args.get<int>(0, 0);
    auto key = text.get_unformatted() + ";;" + std::to_string(power);
    auto& state = states[key];

    if (state.displacements.empty()) {
        state.displacements.resize(text.length());
        state.game_paused = game.is_paused();
    }

    static int ms_between_refresh = 1000 /
        Configurations::get<int>("graphics.canvas-fps", "debug.canvas-fps");
    bool time_to_update = state.ticks(game) - state.last_shake > ms_between_refresh;

    static std::mt19937 engine;
    static std::uniform_int_distribution<int> displacement(-1, 1);
    static std::uniform_int_distribution<int> shake_chance(1, 100);

    if (time_to_update) {
        state.last_shake = state.ticks(game);
        state.shake = shake_chance(engine) > power;
    }

    for (auto i = text.begin(); i != text.end(); ++i) {
        if (!state.shake) {
            decorator.push_text(*i);
            continue;
        }

        auto pos = std::distance(text.begin(), i);
        if (time_to_update) {
            state.displacements[pos].x = displacement(engine);
            state.displacements[pos].y = displacement(engine);
        }

        decorator.push_position(state.displacements[pos]);
        decorator.push_text(*i);
        decorator.pop_position();
    }
}

int Shake_Decorator::State::ticks(Game& game) const {
    // We still want to shake texts created when game is paused
    return game_paused ? game.window_ticks() : game.ticks();
}
