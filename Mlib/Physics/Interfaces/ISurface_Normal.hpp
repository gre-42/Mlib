#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <typename TData, size_t... tshape>
class FixedArray;

class ISurfaceNormal {
public:
	virtual FixedArray<float, 3> get_surface_normal(
		const TransformationMatrix<float, ScenePos, 3>& trafo,
		const FixedArray<ScenePos, 3>& position) const = 0;
};

}
