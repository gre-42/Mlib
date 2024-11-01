#pragma once
#include <Mlib/Map/Verbose_Unordered_Map.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace Mlib {

struct PssgAttribute {
    std::string name;
};

struct PssgNode {
    std::string name;
    VerboseUnorderedMap<uint32_t, PssgAttribute> attributes = { "PSSG attribute", [](uint32_t id) { return std::to_string(id); } };
};

struct PssgTexture {
    std::vector<uint8_t> data;
};

struct PssgSchema {
    VerboseUnorderedMap<uint32_t, PssgNode> nodes = { "PSSG node", [](uint32_t id) { return std::to_string(id); } };
};

struct PssgModel {
    PssgSchema schema;
    std::unordered_map<VariableAndHash<std::string>, PssgTexture> textures;
};

}
