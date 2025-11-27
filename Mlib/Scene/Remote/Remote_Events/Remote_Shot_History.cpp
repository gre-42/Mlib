#include "Remote_Shot_History.hpp"
#include <Mlib/Io/Binary_Reader.hpp>
#include <Mlib/Io/Binary_Writer.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmission_History.hpp>

using namespace Mlib;

Player::ShotHistory Mlib::read_shot_history(
    std::istream& istr,
    TransmissionHistoryReader& transmission_history_reader,
    IoVerbosity verbosity)
{
    Player::ShotHistory result;
    BinaryReader reader{ istr, verbosity };
    auto nevents = reader.read_binary<uint16_t>("nshots");
    for (uint16_t i = 0; i < nevents; ++i) {
        auto time = transmission_history_reader.read_time(istr, verbosity);
        auto weapon = reader.read_string("weapon name");
        result.try_emplace(time, std::move(weapon));
    }
    return result;
}

void Mlib::write_shot_history(
    const Player::ShotHistory& events,
    std::ostream& ostr,
    TransmissionHistoryWriter& transmission_history_writer)
{
    BinaryWriter writer{ ostr };
    writer.write_binary(integral_cast<uint16_t>(events.size()), "nshots");
    for (const auto& [time, weapon] : events) {
        transmission_history_writer.write_time(ostr, time);
        writer.write_string(weapon, "weapon name");
    }
}
