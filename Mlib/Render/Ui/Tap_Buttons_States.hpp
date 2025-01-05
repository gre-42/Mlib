#pragma once
#include <Mlib/Render/Ui/Tap_Button_State.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <cstddef>
#include <list>
#include <mutex>
#include <unordered_map>

namespace Mlib {

struct TapButtonsStates {
    std::list<TapButtonState> button_states;
    std::unordered_map<int32_t, const TapButtonState*> pointer_ids;
    std::unordered_map<int, bool> button_down;
    std::unordered_map<int, float> joystick_axis_position;
    mutable SafeAtomicRecursiveSharedMutex mutex;
    inline void clear() {
        std::scoped_lock lock{mutex};
        button_states.clear();
    }
};

}
