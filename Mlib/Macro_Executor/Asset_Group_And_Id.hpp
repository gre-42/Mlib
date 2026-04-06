#pragma once
#include <Mlib/Hashing/Hash.hpp>
#include <Mlib/Hashing/Std_Hash.hpp>
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace Mlib {

struct AssetGroupAndId {
    VariableAndHash<std::string> group;
    VariableAndHash<std::string> id;
};

void from_json(const nlohmann::json& j, AssetGroupAndId& gid);

}

template<>
struct std::hash<Mlib::AssetGroupAndId>
{
    std::size_t operator() (const Mlib::AssetGroupAndId& k) const {
        return Mlib::hash_combine(k.group.hash(), k.id.hash());
    }
};
