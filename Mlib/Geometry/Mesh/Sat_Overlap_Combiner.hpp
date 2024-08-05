#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>
#include <set>

namespace Mlib {

struct CollisionRidgeSphere;
template <class TData, size_t tshape0, size_t... tshape>
class OrderableFixedArray;

class SatOverlapCombiner {
public:
	SatOverlapCombiner(
		const std::set<OrderableFixedArray<ScenePos, 3>>& vertices0,
		const std::set<OrderableFixedArray<ScenePos, 3>>& vertices1);
	void combine_sticky_ridge(const CollisionRidgeSphere& e1, ScenePos max_keep_normal);
	void combine_ridges(const CollisionRidgeSphere& e0, const CollisionRidgeSphere& e1);
	void combine_plane(const FixedArray<ScenePos, 3>& normal);
	inline const FixedArray<ScenePos, 3>& best_normal() const {
		return best_normal_;
	}
	inline ScenePos best_min_overlap() const {
		return best_min_overlap_;
	}
private:
	ScenePos overlap_signed(const FixedArray<ScenePos, 3>& normal) const;
	void overlap_unsigned(
		const FixedArray<ScenePos, 3>& normal,
		ScenePos& overlap0,
		ScenePos& overlap1) const;
	bool keep_normal_;
	FixedArray<ScenePos, 3> best_normal_;
	ScenePos best_min_overlap_;

	const std::set<OrderableFixedArray<ScenePos, 3>>& vertices0_;
	const std::set<OrderableFixedArray<ScenePos, 3>>& vertices1_;
};

}