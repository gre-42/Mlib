#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Map/Verbose_Unordered_Map.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <cstdint>
#include <functional>
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

struct PssgSchema {
    VerboseUnorderedMap<uint32_t, PssgNodeInfo> nodes = { "PSSG node info", [](uint32_t id) { return std::to_string(id); } };
    VerboseUnorderedMap<uint32_t, PssgAttributeInfo> attributes = { "PSSG node attribute info", [](uint32_t id) { return std::to_string(id); } };
};

struct PssgAttribute {
    std::vector<std::byte> data;
    std::string string() const;
    uint32_t uint32() const;
    uint64_t uint64() const;
    float float32() const;
    FixedArray<double, 2> dvec2() const;
};

struct PssgNode {
    uint32_t type_id;
    std::vector<std::byte> data;
    std::list<PssgNode> children;
    VerboseUnorderedMap<uint32_t, PssgAttribute> attributes = { "PSSG node attribute info", [](uint32_t id) { return std::to_string(id); } };
    const PssgNode& get_child(const std::string& type, const PssgSchema& schema) const;
    uint32_t nchildren(const std::string& type, const PssgSchema& schema) const;
    const PssgAttribute& get_attribute(const std::string& name, const PssgSchema& schema) const;
    bool has_attribute(const std::string& name, const PssgSchema& schema) const;
    bool for_each_node(const std::function<bool(const PssgNode& node)>& op) const;
    std::string pnstring() const;
    template <class TData>
    TData scalar() const;
    template <class TData, size_t... tshape>
    FixedArray<TData, tshape...> array() const;
    AxisAlignedBoundingBox<float, 3> saabb3() const;
    std::vector<std::byte> texture(const PssgSchema& schema) const;
};

struct PssgModel {
    PssgSchema schema;
    PssgNode root;
};

}
