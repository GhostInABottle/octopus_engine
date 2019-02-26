#ifndef HPP_SHAKE_DECORATOR
#define HPP_SHAKE_DECORATOR

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
};

#endif