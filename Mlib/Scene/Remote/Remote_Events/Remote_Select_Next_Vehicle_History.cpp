#include "Remote_Select_Next_Vehicle_History.hpp"
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Os/Io/Binary_Reader.hpp>
#include <Mlib/Os/Io/Binary_Writer.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Seat.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmission_History.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>

using namespace Mlib;

Player::SelectNextVehicleHistory Mlib::read_select_next_vehicle_history(
    BinaryBitwiseWordsReader& reader,
    TransmissionHistoryReader& transmission_history_reader)
{
    Player::SelectNextVehicleHistory result;
    auto nevents = reader.read_binary<NSelectNextVehicleEventsType>("nselect_next_vehicle_events");
    for (NSelectNextVehicleEventsType i = 0; i < nevents; ++i) {
        auto time = transmission_history_reader.read_time(reader);
        auto q = reader.read_bits<SelectNextVehicleQuery>(SELECT_NEXT_VEHICLE_QUERY_NBITS, "select_next_vehicle_query");
        auto seat = vehicle_seat_to_string(reader.read_bits<VehicleSeat>(VEHICLE_SEAT_NBITS, "seat"));
        result.try_emplace(time, SelectNextVehicleEvent{q, std::move(seat)});
    }
    return result;
}

void Mlib::write_select_next_vehicle_history(
    const Player::SelectNextVehicleHistory& events,
    BinaryBitwiseWordsWriter& writer,
    TransmissionHistoryWriter& transmission_history_writer)
{
    writer.write_binary(integral_cast<NSelectNextVehicleEventsType>(events.size()), "nselect_next_vehicle_events");
    for (const auto& [time, event] : events) {
        transmission_history_writer.write_time(writer, time);
        writer.write_bits(event.q, SELECT_NEXT_VEHICLE_QUERY_NBITS, "select_next_vehicle_query");
        writer.write_bits(vehicle_seat_from_string(event.seat), VEHICLE_SEAT_NBITS, "seat");
    }
}
