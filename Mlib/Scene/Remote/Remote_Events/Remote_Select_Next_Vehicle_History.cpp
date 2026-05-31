#include "Remote_Select_Next_Vehicle_History.hpp"
#include <Mlib/Os/Io/Binary_Reader.hpp>
#include <Mlib/Os/Io/Binary_Writer.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmission_History.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>

using namespace Mlib;

Player::SelectNextVehicleHistory Mlib::read_select_next_vehicle_history(
    std::istream& istr,
    TransmissionHistoryReader& transmission_history_reader,
    IoVerbosity verbosity)
{
    Player::SelectNextVehicleHistory result;
    BinaryReader reader{ istr, verbosity };
    auto nevents = reader.read_binary<NSelectNextVehicleEventsType>("nselect_next_vehicle_events");
    for (NSelectNextVehicleEventsType i = 0; i < nevents; ++i) {
        auto time = transmission_history_reader.read_time(istr, verbosity);
        auto q = reader.read_binary<SelectNextVehicleQuery>("select_next_vehicle_query");
        auto seat = reader.read_string<StringLengthType>("seat");
        result.try_emplace(time, SelectNextVehicleEvent{q, std::move(seat)});
    }
    return result;
}

void Mlib::write_select_next_vehicle_history(
    const Player::SelectNextVehicleHistory& events,
    std::ostream& ostr,
    TransmissionHistoryWriter& transmission_history_writer)
{
    BinaryWriter writer{ ostr };
    writer.write_binary(integral_cast<NSelectNextVehicleEventsType>(events.size()), "nselect_next_vehicle_events");
    for (const auto& [time, event] : events) {
        transmission_history_writer.write_time(ostr, time);
        writer.write_binary(event.q, "select_next_vehicle_query");
        writer.write_string<StringLengthType>(event.seat, "seat");
    }
}
