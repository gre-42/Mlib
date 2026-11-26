#pragma once
#include <Mlib/Geometry/Primitives/Bvh.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <list>

namespace Mlib {

template <class TPos>
struct ColoredVertex;
class PTri;

template <class EntityType>
class RegionBvh {
    struct RegionTriangle {
        FixedArray<CompressedScenePos, 3, 2> triangle;
        EntityType entity_type;
    };
public:
    explicit RegionBvh();
    ~RegionBvh();
    void add(const FixedArray<CompressedScenePos, 3, 2>& triangle, EntityType entity_type);
    std::optional<EntityType> entity_type(const FixedArray<CompressedScenePos, 2>& pt) const;
    std::optional<EntityType> entity_type(const PTri& tri) const;
private:
    Bvh<CompressedScenePos, 2, RegionTriangle> bvh_;
};

}
