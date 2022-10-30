#pragma once

namespace Mlib {

enum class ScoreBoardConfiguration {
    NONE = 0,
    TEAM = (1 << 0),
    BEST_LAP_TIME = (1 << 1),
    CAR_HP = (1 << 2),
    HISTORY = (1 << 3),
    NWINS = (1 << 4),
    NKILLS = (1 << 5),
    LAPS = (1 << 6),
    RANK = (1 << 7),
    RACE_TIME = (1 << 8)
};

inline ScoreBoardConfiguration operator & (ScoreBoardConfiguration a, ScoreBoardConfiguration b) {
    return (ScoreBoardConfiguration)((int)a & (int)b);
}

inline bool any(ScoreBoardConfiguration c) {
    return c != ScoreBoardConfiguration::NONE;
}

}
