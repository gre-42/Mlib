#pragma once
#include <cstddef>

namespace Mlib {

struct IntersectionScene;
template <class TData, size_t tndim>
class PlaneNd;
template <typename TData, size_t... tshape>
class FixedArray;

bool compute_edge_overlap(
	const IntersectionScene& c,
	const FixedArray<double, 3>& intersection_point,
	const PlaneNd<double, 3>& N0_f,
	bool& sat_used,
	double& overlap,
	FixedArray<double, 3>& normal);

}
