#pragma once
#include <Mlib/Render/Ui/Tap_Button_State.hpp>
#include <map>
#include <mutex>
#include <shared_mutex>

namespace Mlib {

struct TapButtonsStates {
    std::map<int, TapButtonState> button_states;
    mutable std::shared_mutex mutex;
    inline void clear() {
        std::unique_lock lock{mutex};
        button_states.clear();
    }
};

}
