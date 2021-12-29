#pragma once

namespace Mlib {

enum class ScoreBoardConfiguration {
    TEAM = (1 << 0),
    BEST_LAP_TIME = (1 << 1),
    CAR_HP = (1 << 2),
    HISTORY = (1 << 3)
};

inline int operator & (ScoreBoardConfiguration a, ScoreBoardConfiguration b) {
    return (int)a & (int)b;
}

}
