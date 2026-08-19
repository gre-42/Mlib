#pragma once
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Remote/Incremental_Objects/Remote_Object_Id.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <compare>
#include <optional>
#include <string>
#include <unordered_set>

namespace Mlib {

class BinaryBitwiseWordsWriter;
class BinaryBitwiseWordsReader;

struct KillEvent {
    VariableAndHash<std::string> player;
    VariableAndHash<std::string> victim_player;
    std::optional<NTeamCountType> team;
    std::optional<NTeamCountType> victim_team;
    std::optional<RemoteObjectId> victim_vehicle_id;
    std::strong_ordering operator <=> (const KillEvent&) const = default;
};

void save(
    BinaryBitwiseWordsWriter& writer,
    const KillEvent& value,
    std::string_view message);

void load(
    BinaryBitwiseWordsReader& reader,
    KillEvent& result,
    std::string_view message);

}

template <>
struct std::hash<Mlib::KillEvent>
{
    inline std::size_t operator() (const Mlib::KillEvent& a) const {
        return Mlib::hash_combine(
            a.player,
            a.victim_player,
            a.team,
            a.victim_team,
            a.victim_vehicle_id);
    }
};
