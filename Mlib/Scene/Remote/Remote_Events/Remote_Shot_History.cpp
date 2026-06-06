#include "Remote_Shot_History.hpp"
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmission_History.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>

using namespace Mlib;

Player::ShotHistory Mlib::read_shot_history(
    BinaryBitwiseWordsReader& reader,
    TransmissionHistoryReader& transmission_history_reader)
{
    Player::ShotHistory result;
    auto nevents = reader.read_binary<NShotsType>("nshots");
    for (NShotsType i = 0; i < nevents; ++i) {
        auto time = transmission_history_reader.read_time(reader);
        auto weapon = reader.read_string<StringLengthType>("weapon name");
        result.try_emplace(time, std::move(weapon));
    }
    return result;
}

void Mlib::write_shot_history(
    const Player::ShotHistory& events,
    BinaryBitwiseWordsWriter& writer,
    TransmissionHistoryWriter& transmission_history_writer)
{
    writer.write_binary(integral_cast<NShotsType>(events.size()), "nshots");
    for (const auto& [time, weapon] : events) {
        transmission_history_writer.write_time(writer, time);
        writer.write_string<StringLengthType>(weapon, "weapon name");
    }
}
