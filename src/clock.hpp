#ifndef HPP_CLOCK
#define HPP_CLOCK

class Game;

class Clock {
public:
    explicit Clock(Game& game);
    // Check if game time is stopped
    bool stopped() const noexcept { return time_stop; }
    // Stop game time
    void stop_time();
    // Resume game time
    void resume_time();
    // Get total time in seconds without stopped time
    float seconds() const;
private:
    Game& game;
    int start_time;
    bool time_stop;
    int stop_start_time;
    int total_stopped_time;
};

#endif
