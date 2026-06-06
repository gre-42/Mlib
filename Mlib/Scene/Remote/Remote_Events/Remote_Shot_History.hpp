#pragma once
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <iosfwd>

namespace Mlib {

class BinaryBitwiseWordsReader;
class BinaryBitwiseWordsWriter;
class TransmissionHistoryReader;
class TransmissionHistoryWriter;

Player::ShotHistory read_shot_history(
    BinaryBitwiseWordsReader& reader,
    TransmissionHistoryReader& transmission_history_reader);

void write_shot_history(
    const Player::ShotHistory& events,
    BinaryBitwiseWordsWriter& writer,
    TransmissionHistoryWriter& transmission_history_writer);

}
