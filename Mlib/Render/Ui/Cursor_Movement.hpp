#pragma once
#include <Mlib/Render/Ui/Incremental_Movement.hpp>
#include <string>

namespace Mlib {

class CursorStates;
class KeyConfigurations;

class CursorMovement {
    CursorMovement& operator = (const CursorMovement&) = delete;
public:
    explicit CursorMovement(
        CursorStates& cursor_states,
        const KeyConfigurations& key_configurations,
        std::string id);
    ~CursorMovement();
    float axis_alpha(float ncached);
private:
    IncrementalMovement incremental_movement_;
    const KeyConfigurations& key_configurations_;
    std::string id_;
};

}
