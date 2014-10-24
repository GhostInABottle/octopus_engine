#ifndef HPP_CLOCK
#define HPP_CLOCK

class Game;

class Clock {
public:
    Clock(Game& game);
    // Get game ticks
    int ticks() const;
    // Check if game time is stopped
    bool stopped() const { return time_stop; }
    // Stop game time
    void stop_time();
    // Resume game time
    void resume_time();
    // Get total time in seconds without applying a multipler
    int actual_seconds() const;
    // Get total time in seconds after applying multiplier
    int total_seconds() const {
        return static_cast<int>(actual_seconds() * time_multiplier);
    }
    void add_seconds(int seconds);
private:
    Game& game;
    int start_time;
    bool time_stop;
    int stop_start_time;
    int total_stopped_time;
    int added_time;
    float time_multiplier;
};

#endif
