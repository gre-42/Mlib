#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <cstddef>
#include <set>

namespace Mlib {

struct CollisionRidgeSphere;
template <class TData, size_t... tshape>
class OrderableFixedArray;

class SatOverlapCombiner {
public:
	SatOverlapCombiner(
		const std::set<OrderableFixedArray<double, 3>>& vertices0,
		const std::set<OrderableFixedArray<double, 3>>& vertices1);
	void combine_sticky_ridge(const CollisionRidgeSphere& e1, double max_keep_normal);
	void combine_ridges(const CollisionRidgeSphere& e0, const CollisionRidgeSphere& e1);
	void combine_plane(const FixedArray<double, 3>& normal);
	inline const FixedArray<double, 3>& best_normal() const {
		return best_normal_;
	}
	inline double best_min_overlap() const {
		return best_min_overlap_;
	}
private:
	double overlap_signed(const FixedArray<double, 3>& normal) const;
	void overlap_unsigned(
		const FixedArray<double, 3>& normal,
		double& overlap0,
		double& overlap1) const;
	bool keep_normal_;
	FixedArray<double, 3> best_normal_;
	double best_min_overlap_;

	const std::set<OrderableFixedArray<double, 3>>& vertices0_;
	const std::set<OrderableFixedArray<double, 3>>& vertices1_;
};

}