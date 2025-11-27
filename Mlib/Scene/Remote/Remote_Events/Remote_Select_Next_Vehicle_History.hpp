#pragma once
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <iosfwd>

namespace Mlib {

class TransmissionHistoryReader;
class TransmissionHistoryWriter;

Player::SelectNextVehicleHistory read_select_next_vehicle_history(
    std::istream& istr,
    TransmissionHistoryReader& transmission_history_reader,
    IoVerbosity verbosity);

void write_select_next_vehicle_history(
    const Player::SelectNextVehicleHistory& events,
    std::ostream& ostr,
    TransmissionHistoryWriter& transmission_history_writer);

}
