#pragma once
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <iosfwd>

namespace Mlib {

class TransmissionHistoryReader;
class TransmissionHistoryWriter;

Player::ShotHistory read_shot_history(
    std::istream& istr,
    TransmissionHistoryReader& transmission_history_reader,
    IoVerbosity verbosity);

void write_shot_history(
    const Player::ShotHistory& events,
    std::ostream& ostr,
    TransmissionHistoryWriter& transmission_history_writer);

}
