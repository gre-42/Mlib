#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <set>

namespace Mlib {

template <class TPosition>
struct CollisionRidgeSphere;
template <class TData, size_t... tshape>
class OrderableFixedArray;

class SatOverlapCombiner {
public:
    SatOverlapCombiner(
        const std::set<OrderableFixedArray<CompressedScenePos, 3>>& vertices0,
        const std::set<OrderableFixedArray<CompressedScenePos, 3>>& vertices1);
    void combine_sticky_ridge(const CollisionRidgeSphere<CompressedScenePos>& e1, ScenePos max_keep_normal);
    void combine_ridges(const CollisionRidgeSphere<CompressedScenePos>& e0, const CollisionRidgeSphere<CompressedScenePos>& e1);
    void combine_plane(const FixedArray<SceneDir, 3>& normal);
    inline const FixedArray<SceneDir, 3>& best_normal() const {
        return best_normal_;
    }
    inline ScenePos best_min_overlap() const {
        return best_min_overlap_;
    }
private:
    ScenePos overlap_signed(const FixedArray<SceneDir, 3>& normal) const;
    void overlap_unsigned(
        const FixedArray<SceneDir, 3>& normal,
        ScenePos& overlap0,
        ScenePos& overlap1) const;
    bool keep_normal_;
    FixedArray<SceneDir, 3> best_normal_;
    ScenePos best_min_overlap_;

    const std::set<OrderableFixedArray<CompressedScenePos, 3>>& vertices0_;
    const std::set<OrderableFixedArray<CompressedScenePos, 3>>& vertices1_;
};

}