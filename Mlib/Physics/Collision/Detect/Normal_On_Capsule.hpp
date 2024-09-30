#pragma once
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Interfaces/ISurface_Normal.hpp>

namespace Mlib {

class NormalOnCapsule: public ISurfaceNormal {
public:
	explicit NormalOnCapsule(
		const TransformationMatrix<float, ScenePos, 3>& trafo);
	virtual FixedArray<float, 3> get_surface_normal(
		const TransformationMatrix<float, ScenePos, 3>& trafo,
		const FixedArray<ScenePos, 3>& position) const override;
private:
	TransformationMatrix<float, ScenePos, 3> itrafo_;
	FixedArray<float, 3, 3> rotate_;
};

}
