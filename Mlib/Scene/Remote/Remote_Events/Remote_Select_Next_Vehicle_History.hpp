#pragma once
#include <Mlib/Players/Advance_Times/Player.hpp>

namespace Mlib {

class BinaryBitwiseWordsReader;
class BinaryBitwiseWordsWriter;
class TransmissionHistoryReader;
class TransmissionHistoryWriter;

Player::SelectNextVehicleHistory read_select_next_vehicle_history(
    BinaryBitwiseWordsReader& reader,
    TransmissionHistoryReader& transmission_history_reader);

void write_select_next_vehicle_history(
    const Player::SelectNextVehicleHistory& events,
    BinaryBitwiseWordsWriter& writer,
    TransmissionHistoryWriter& transmission_history_writer);

}
