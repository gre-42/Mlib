#pragma once
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations_Fwd.hpp>
#include <Mlib/Render/Ui/Incremental_Movement.hpp>
#include <string>

namespace Mlib {

class CursorStates;

class ScrollWheelMovement {
    ScrollWheelMovement& operator = (const ScrollWheelMovement&) = delete;
public:
    explicit ScrollWheelMovement(
        CursorStates& cursor_states,
        const LockableKeyConfigurations& key_configurations,
        std::string id);
    ~ScrollWheelMovement();
    float axis_alpha(float ncached);
private:
    IncrementalMovement incremental_movement_;
    const LockableKeyConfigurations& key_configurations_;
    std::string id_;
};

}
