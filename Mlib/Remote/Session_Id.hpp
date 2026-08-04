#pragma once
#include <Mlib/Scene_Config/Remote_Integers.hpp>

namespace Mlib {

inline GenericSessionIdType get_session_id(GenericSessionIdType previous) {
    while (true) {
        auto now = std::chrono::system_clock::now();
        auto duration = (GenericSessionIdType)std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        if ((duration != 0) && (duration != previous)) {
            return duration;
        }
    }
}

}
