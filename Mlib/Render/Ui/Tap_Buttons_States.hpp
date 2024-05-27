#pragma once
#include <Mlib/Render/Ui/Tap_Button_State.hpp>
#include <Mlib/Threads/Safe_Shared_Mutex.hpp>
#include <list>
#include <map>
#include <mutex>

namespace Mlib {

struct TapButtonsStates {
    std::list<TapButtonState> button_states;
    std::map<int, bool> button_down;
    std::map<int, float> joystick_axis_position;
    mutable SafeSharedMutex mutex;
    inline void clear() {
        std::scoped_lock lock{mutex};
        button_states.clear();
    }
};

}
