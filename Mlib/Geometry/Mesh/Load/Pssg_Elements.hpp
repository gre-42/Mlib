#pragma once
#include <Mlib/Map/Verbose_Unordered_Map.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <cstdint>
#include <list>
#include <string>
#include <unordered_map>

namespace Mlib {

struct PssgAttributeInfo {
    std::string name;
};

struct PssgNodeInfo {
    std::string name;
    std::list<uint32_t> attributes;
};

struct PssgTexture {
    std::vector<uint8_t> data;
};

struct PssgSchema {
    VerboseUnorderedMap<uint32_t, PssgNodeInfo> nodes = { "PSSG node info", [](uint32_t id) { return std::to_string(id); } };
    VerboseUnorderedMap<uint32_t, PssgAttributeInfo> attributes = { "PSSG node attribute info", [](uint32_t id) { return std::to_string(id); } };
};

struct PssgAttribute {
    std::vector<uint8_t> data;
    std::string string() const;
};

struct PssgNode {
    std::vector<uint8_t> data;
    std::list<PssgNode> children;
    VerboseUnorderedMap<uint32_t, PssgAttribute> attributes = { "PSSG node attribute info", [](uint32_t id) { return std::to_string(id); } };
    std::string pnstring() const;
};

struct PssgModel {
    PssgSchema schema;
    PssgNode root;
    std::unordered_map<VariableAndHash<std::string>, PssgTexture> textures;
};

}
