#pragma once
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Extended_Image.hpp>

namespace Mlib {

class HeightSampler {
public:
	HeightSampler(
		ExtendedImage image,
		const TransformationMatrix<double, double, 2>& normalization_matrix);
	bool operator () (const FixedArray<double, 2>& pos, double& z) const;
private:
	ExtendedImage image_;
	TransformationMatrix<double, double, 2> normalization_matrix_;
};

}
