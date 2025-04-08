#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

namespace Mlib {

enum class InteriorTextureSet: uint32_t;

static const size_t INTERIOR_LEFT = 0;
static const size_t INTERIOR_RIGHT = 1;
static const size_t INTERIOR_FLOOR = 2;
static const size_t INTERIOR_CEILING = 3;
static const size_t INTERIOR_BACK = 4;
// back_specular        5
// front_color          6
// front_alpha          7
// front_specular       8
static const size_t INTERIOR_COUNT_MAX = 9;

struct InteriorTextures {
    InteriorTextures();
    InteriorTextures(const InteriorTextures& other);
    InteriorTextures(InteriorTextures&& other);
    InteriorTextures& operator = (const InteriorTextures& other);
    InteriorTextures& operator = (InteriorTextures&& other);
    std::strong_ordering operator <=> (const InteriorTextures&) const = default;
    bool empty() const;
    size_t size() const;
    const VariableAndHash<std::string>& operator [](size_t index) const;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(facade_edge_size);
        archive(facade_inner_size);
        archive(interior_size);
        archive(names);
    }
    OrderableFixedArray<float, 2> facade_edge_size{ 0.f, 0.f };
    OrderableFixedArray<float, 2> facade_inner_size{ 0.f, 0.f };
    OrderableFixedArray<float, 3> interior_size{ 0.f, 0.f, 0.f };
    std::vector<VariableAndHash<std::string>> names;
    InteriorTextureSet set;
    void assign(
        VariableAndHash<std::string> left,
        VariableAndHash<std::string> right,
        VariableAndHash<std::string> floor,
        VariableAndHash<std::string> ceiling,
        VariableAndHash<std::string> back,
        VariableAndHash<std::string> back_specular,
        VariableAndHash<std::string> front_color,
        VariableAndHash<std::string> front_alpha,
        VariableAndHash<std::string> front_specular);
};

std::ostream& operator << (std::ostream& ostr, const InteriorTextures& t);

}
