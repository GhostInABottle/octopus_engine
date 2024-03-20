#ifndef HPP_SHOW_POSE_COMMAND
#define HPP_SHOW_POSE_COMMAND

#include <string>
#include "../direction.hpp"
#include "../commands/command.hpp"

class Map;
class Sprite_Holder;
class Sprite;

class Show_Pose_Command : public Command {
public:
    enum class Holder_Type { MAP_OBJECT, LAYER, CANVAS };
    struct Holder_Info {
        Holder_Type type;
        int id;
        int parent_id;
        Holder_Info(Holder_Type type, int id, int parent_id = -1)
            : type(type), id(id), parent_id(parent_id) {}
    };
    Show_Pose_Command(Map& map, Holder_Info holder_info, const std::string& pose_name,
        const std::string& state = "", Direction dir = Direction::NONE);
    void execute() override;
    bool is_complete() const override;
    void pause() override;
    void resume() override;
private:
    Sprite_Holder* get_holder() const;
    Holder_Info holder_info;
    bool complete;
};

#endif