#pragma once
#include <Mlib/Render/Ui/Tap_Button_State.hpp>
#include <Mlib/Threads/Safe_Shared_Mutex.hpp>
#include <map>
#include <mutex>

namespace Mlib {

struct TapButtonsStates {
    std::map<int, TapButtonState> button_states;
    mutable SafeSharedMutex mutex;
    inline void clear() {
        std::scoped_lock lock{mutex};
        button_states.clear();
    }
};

}
