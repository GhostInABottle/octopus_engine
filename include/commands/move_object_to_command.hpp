#ifndef HPP_MOVE_OBJECT_TO_COMMAND
#define HPP_MOVE_OBJECT_TO_COMMAND

#include <memory>
#include "../command.hpp"
#include "../collision_check_types.hpp"

class Map_Object;
class Map;

class Move_Object_To_Command : public Command {
public:
    Move_Object_To_Command(Map& map, Map_Object& object, float x, float y,
        Collision_Check_Type check_type = Collision_Check_Type::BOTH,
        bool keep_trying = false);
    ~Move_Object_To_Command();
    void execute() override;
    bool is_complete() const noexcept override;
private:
    struct Impl;
    friend struct Impl;
    std::unique_ptr<Impl> pimpl;
};

#endif