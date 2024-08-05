#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>

namespace Mlib {

struct IntersectionScene;
template <class TData, size_t tndim>
class PlaneNd;
struct CollisionRidgeSphere;
template <typename TData, size_t... tshape>
class FixedArray;

bool compute_edge_overlap(
	const IntersectionScene& c,
	const FixedArray<ScenePos, 3>& intersection_point,
	bool& sat_used,
	ScenePos& overlap,
	FixedArray<ScenePos, 3>& normal);

}
