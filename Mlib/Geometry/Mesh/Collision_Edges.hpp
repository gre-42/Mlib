#pragma once
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <cstdint>
#include <set>

namespace Mlib {

template <class TData, size_t... tshape>
class OrderableFixedArray;
enum class PhysicsMaterial: uint32_t;

struct OrderableEdgeSphere {
    CollisionLineSphere<CompressedScenePos> collision_line_sphere;
    std::pair<OrderableFixedArray<CompressedScenePos, 3>, OrderableFixedArray<CompressedScenePos, 3>> key() const;
    bool operator < (const OrderableEdgeSphere& other) const;
};

class CollisionEdges {
public:
    CollisionEdges();
    ~CollisionEdges();
    using Edges = std::set<OrderableEdgeSphere>;
    using const_iterator = Edges::const_iterator;
    template <size_t tnvertices>
    void insert(
        const FixedArray<CompressedScenePos, tnvertices, 3>& tri,
        PhysicsMaterial physics_material);
    const_iterator begin() const;
    const_iterator end() const;
    size_t size() const;
private:
    void insert(
        const FixedArray<CompressedScenePos, 3>& a,
        const FixedArray<CompressedScenePos, 3>& b,
        PhysicsMaterial physics_material);
    Edges edges_;
};

}
