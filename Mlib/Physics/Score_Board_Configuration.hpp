#pragma once
#include <string>

namespace Mlib {

enum class ScoreBoardConfiguration {
    NONE            = 0,
    USER            = (1 << 0),
    PLAYER          = (1 << 1),
    TEAM            = (1 << 2),
    BEST_LAP_TIME   = (1 << 3),
    CAR_HP          = (1 << 4),
    HISTORY         = (1 << 5),
    NWINS           = (1 << 6),
    NKILLS          = (1 << 7),
    LAPS            = (1 << 8),
    RANK            = (1 << 9),
    RACE_TIME       = (1 << 10)
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
