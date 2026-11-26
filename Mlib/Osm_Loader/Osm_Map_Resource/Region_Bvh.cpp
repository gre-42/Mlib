#include "Region_Bvh.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Primitives/Point_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Triangle_Is_Right_Handed.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/PTri.hpp>
#include <Mlib/Stats/Mean.hpp>

namespace Mlib {
enum class TerrainType: uint32_t;
enum class WaterType: uint32_t;
}

using namespace Mlib;

template <class EntityType>
RegionBvh<EntityType>::RegionBvh()
    : bvh_{ {(CompressedScenePos)100.f, (CompressedScenePos)100.f}, 10 }
{}

template <class EntityType>
RegionBvh<EntityType>::~RegionBvh() = default;

template <class EntityType>
void RegionBvh<EntityType>::add(
    const FixedArray<CompressedScenePos, 3, 2>& triangle,
    EntityType entity_type)
{
    if (!triangle_is_right_handed(funpack(triangle))) {
        throw std::runtime_error("Triangle is not right-handed");
    }
    bvh_.insert(AxisAlignedBoundingBox<CompressedScenePos, 2>::from_points(triangle), RegionTriangle{triangle, entity_type});
}

template <class EntityType>
std::optional<EntityType> RegionBvh<EntityType>::entity_type(const FixedArray<CompressedScenePos, 2>& pt) const
{
    const RegionTriangle* nearest_payload;
    auto dist = bvh_.min_distance(
        pt,
        (CompressedScenePos)0.f, // max_dist
        [&pt](const RegionTriangle& rt) {
            return (CompressedScenePos)distance_point_to_triangle(funpack(pt), funpack(rt.triangle));
        },
        &nearest_payload);
    if (!dist.has_value()) {
        return std::nullopt;
    }
    return nearest_payload->entity_type;
}

template <class EntityType>
std::optional<EntityType> RegionBvh<EntityType>::entity_type(const PTri& tri) const {
    auto t = FixedArray<double, 3, 2>{
        FixedArray<double, 2>(tri[0]->x, tri[0]->y),
        FixedArray<double, 2>(tri[1]->x, tri[1]->y),
        FixedArray<double, 2>(tri[2]->x, tri[2]->y)};
    auto center = mean<0>(t);
    return entity_type(center.casted<CompressedScenePos>());
}

template class Mlib::RegionBvh<TerrainType>;
template class Mlib::RegionBvh<WaterType>;
