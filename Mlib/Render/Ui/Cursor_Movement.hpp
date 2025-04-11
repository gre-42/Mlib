#pragma once
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations_Fwd.hpp>
#include <Mlib/Render/Ui/Incremental_Movement.hpp>
#include <string>

namespace Mlib {

class CursorStates;

class CursorMovement {
    CursorMovement& operator = (const CursorMovement&) = delete;
public:
    explicit CursorMovement(
        CursorStates& cursor_states,
        const LockableKeyConfigurations& key_configurations,
        std::string id);
    ~CursorMovement();
    float axis_alpha(float ncached);
private:
    IncrementalMovement incremental_movement_;
    const LockableKeyConfigurations& key_configurations_;
    std::string id_;
};

}
