#include <random>
#include "../include/game.hpp"
#include "../include/shake_decorator.hpp"

Shake_Decorator::Shake_Decorator(Game& game) : game(game) {}

void Shake_Decorator::operator()(xd::text_decorator& decorator, const xd::formatted_text& text, const xd::text_decorator_args& args) {
    static std::mt19937 engine;
    static std::uniform_int_distribution<int> displacement(-1, 1);
    static std::uniform_int_distribution<int> shake_chance(1, 100);

    auto power = 100 - args.get<int>(0, 0);
    auto shake = shake_chance(engine) > power;

    for (auto i = text.begin(); i != text.end(); ++i) {
        if (shake) {
            decorator.push_position(glm::ivec2(displacement(engine), displacement(engine)));
            decorator.push_text(*i);
            decorator.pop_position();
        } else {
            decorator.push_text(*i);
        }
    }
}