#pragma once
#include <string>

namespace Mlib {

enum class ScoreBoardConfiguration {
    NONE = 0,
    PLAYER = (1 << 0),
    TEAM = (1 << 1),
    BEST_LAP_TIME = (1 << 2),
    CAR_HP = (1 << 3),
    HISTORY = (1 << 4),
    NWINS = (1 << 5),
    NKILLS = (1 << 6),
    LAPS = (1 << 7),
    RANK = (1 << 8),
    RACE_TIME = (1 << 9)
};

inline ScoreBoardConfiguration operator & (ScoreBoardConfiguration a, ScoreBoardConfiguration b) {
    return (ScoreBoardConfiguration)((int)a & (int)b);
}

inline ScoreBoardConfiguration operator | (ScoreBoardConfiguration a, ScoreBoardConfiguration b) {
    return (ScoreBoardConfiguration)((unsigned int)a | (unsigned int)b);
}

inline ScoreBoardConfiguration& operator |= (ScoreBoardConfiguration& a, ScoreBoardConfiguration b) {
    a = a | b;
    return a;
}

inline bool any(ScoreBoardConfiguration c) {
    return c != ScoreBoardConfiguration::NONE;
}

ScoreBoardConfiguration score_board_configuration_from_string(const std::string& s);

}
