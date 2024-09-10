#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <iosfwd>
#include <string>

namespace Mlib {

static const size_t INTERIOR_LEFT = 0;
static const size_t INTERIOR_RIGHT = 1;
static const size_t INTERIOR_FLOOR = 2;
static const size_t INTERIOR_CEILING = 3;
static const size_t INTERIOR_BACK = 4;
static const size_t INTERIOR_COUNT = 5;

struct InteriorTextures {
    InteriorTextures();
    InteriorTextures(const InteriorTextures& other);
    InteriorTextures(InteriorTextures&& other);
    InteriorTextures& operator = (const InteriorTextures& other);
    InteriorTextures& operator = (InteriorTextures&& other);
    std::strong_ordering operator <=> (const InteriorTextures&) const = default;
    bool empty() const;
    const VariableAndHash<std::string>& operator [](size_t index) const;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(facade_edge_size);
        archive(facade_inner_size);
        archive(interior_size);

        archive(left);
        archive(right);
        archive(floor);
        archive(ceiling);
        archive(back);
    }
    OrderableFixedArray<float, 2> facade_edge_size{ 0.f, 0.f };
    OrderableFixedArray<float, 2> facade_inner_size{ 0.f, 0.f };
    OrderableFixedArray<float, 3> interior_size{ 0.f, 0.f, 0.f };
    VariableAndHash<std::string> left;
    VariableAndHash<std::string> right;
    VariableAndHash<std::string> floor;
    VariableAndHash<std::string> ceiling;
    VariableAndHash<std::string> back;
};

std::ostream& operator << (std::ostream& ostr, const InteriorTextures& t);

}
