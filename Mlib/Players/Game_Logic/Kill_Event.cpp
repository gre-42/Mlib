#include "Kill_Event.hpp"
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Os/Io/Serialize/Serialize.hpp>
#include <Mlib/Scene_Config/Remote_Transmission.hpp>

using namespace Mlib;

void Mlib::save(
    BinaryBitwiseWordsWriter& writer,
    const KillEvent& value,
    std::string_view message)
{
    writer.write_bool_bit(value.player->empty(), "player empty");
    writer.write_bool_bit(value.victim_player->empty(), "victim player empty");
    writer.write_bool_bit(value.team.has_value(), "team has value");
    writer.write_bool_bit(value.victim_team.has_value(), "victim team has value");
    writer.write_bool_bit(value.victim_vehicle_id.has_value(), "victim team has value");
    if (!value.player->empty()) {
        writer.write_string<StringLengthType>(*value.player, "player");
    }
    if (!value.victim_player->empty()) {
        writer.write_string<StringLengthType>(*value.victim_player, "victim player");
    }
    if (value.team.has_value()) {
        writer.write_binary(*value.team, "team");
    }
    if (value.victim_team.has_value()) {
        writer.write_binary(*value.victim_team, "victim team");
    }
    if (value.victim_vehicle_id.has_value()) {
        writer.serialize(*value.victim_vehicle_id, "victim vehicle");
    }
}

void Mlib::load(
    BinaryBitwiseWordsReader& reader,
    KillEvent& result,
    std::string_view message)
{
    bool player_empty = reader.read_bool_bit("player empty");
    bool victim_player_empty = reader.read_bool_bit("victim player empty");
    bool team_has_value = reader.read_bool_bit("team has value");
    bool victim_team_has_value = reader.read_bool_bit("victim team has value");
    bool victim_vehicle_has_value = reader.read_bool_bit("victim team has value");
    if (!player_empty) {
        result.player = reader.read_string<StringLengthType>("player");
    }
    if (!victim_player_empty) {
        result.victim_player = reader.read_string<StringLengthType>("victim player");
    }
    if (team_has_value) {
        result.team.emplace(reader.read_binary<NTeamCountType>("team"));
    }
    if (victim_team_has_value) {
        result.victim_team = reader.read_binary<NTeamCountType>("victim team");
    }
    if (victim_vehicle_has_value) {
        result.victim_vehicle_id.emplace(reader.deserialize<RemoteObjectId>("victim team"));
    }
}
