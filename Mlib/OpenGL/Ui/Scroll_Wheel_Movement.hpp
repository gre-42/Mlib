#pragma once
#include <Mlib/OpenGL/Key_Bindings/Lockable_Key_Configurations_Fwd.hpp>
#include <Mlib/OpenGL/Ui/Incremental_Movement.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <cstdint>
#include <string>

namespace Mlib {

class CursorStates;

class ScrollWheelMovement {
    ScrollWheelMovement& operator = (const ScrollWheelMovement&) = delete;
public:
    explicit ScrollWheelMovement(
        CursorStates& cursor_states,
        const LockableKeyConfigurations& key_configurations,
        NUserCountType user_id,
        std::string id);
    ~ScrollWheelMovement();
    float axis_alpha(float ncached);
private:
    IncrementalMovement incremental_movement_;
    const LockableKeyConfigurations& key_configurations_;
    NUserCountType user_id_;
    std::string id_;
};

}
