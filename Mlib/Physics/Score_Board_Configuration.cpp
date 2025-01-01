#include "Score_Board_Configuration.hpp"
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>
#include <stdexcept>

using namespace Mlib;

static ScoreBoardConfiguration single_score_board_configuration_from_string(const std::string& s) {
    static const std::map<std::string, ScoreBoardConfiguration> m{
        {"none", ScoreBoardConfiguration::NONE},
        {"player", ScoreBoardConfiguration::PLAYER},
        {"team", ScoreBoardConfiguration::TEAM},
        {"best_lap_time", ScoreBoardConfiguration::BEST_LAP_TIME},
        {"car_hp", ScoreBoardConfiguration::CAR_HP},
        {"history", ScoreBoardConfiguration::HISTORY},
        {"nwins", ScoreBoardConfiguration::NWINS},
        {"nkills", ScoreBoardConfiguration::NKILLS},
        {"laps", ScoreBoardConfiguration::LAPS},
        {"rank", ScoreBoardConfiguration::RANK},
        {"race_time", ScoreBoardConfiguration::RACE_TIME},
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown score board configuration: \"" + s + '"');
    }
    return it->second;
}

ScoreBoardConfiguration Mlib::score_board_configuration_from_string(const std::string& s) {
    static const DECLARE_REGEX(re, "\\|");
    ScoreBoardConfiguration result = ScoreBoardConfiguration::NONE;
    for (const auto& m : string_to_list(s, re)) {
        result |= single_score_board_configuration_from_string(m);
    }
    return result;
}
