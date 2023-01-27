#include "Score_Board_Configuration.hpp"
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stdexcept>

using namespace Mlib;

static ScoreBoardConfiguration single_score_board_configuration_from_string(const std::string& s) {
    if (s == "none") {
        return ScoreBoardConfiguration::NONE;
    }
    if (s == "team") {
        return ScoreBoardConfiguration::TEAM;
    }
    if (s == "best_lap_time") {
        return ScoreBoardConfiguration::BEST_LAP_TIME;
    }
    if (s == "car_hp") {
        return ScoreBoardConfiguration::CAR_HP;
    }
    if (s == "history") {
        return ScoreBoardConfiguration::HISTORY;
    }
    if (s == "nwins") {
        return ScoreBoardConfiguration::NWINS;
    }
    if (s == "nkills") {
        return ScoreBoardConfiguration::NKILLS;
    }
    if (s == "laps") {
        return ScoreBoardConfiguration::LAPS;
    }
    if (s == "rank") {
        return ScoreBoardConfiguration::RANK;
    }
    if (s == "race_time") {
        return ScoreBoardConfiguration::RACE_TIME;
    }
    THROW_OR_ABORT("Unknown score board configuration: \"" + s + '"');
}

ScoreBoardConfiguration Mlib::score_board_configuration_from_string(const std::string& s) {
    static const DECLARE_REGEX(re, "\\|");
    ScoreBoardConfiguration result = ScoreBoardConfiguration::NONE;
    for (const auto& m : string_to_list(s, re)) {
        result |= single_score_board_configuration_from_string(m);
    }
    return result;
}
