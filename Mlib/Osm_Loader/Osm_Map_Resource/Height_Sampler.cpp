#include "Height_Sampler.hpp"

using namespace Mlib;

HeightSampler::HeightSampler(
	ExtendedImage image,
	const TransformationMatrix<double, double, 2>& normalization_matrix)
	: image_{ std::move(image) }
	, normalization_matrix_{ normalization_matrix }
{}

bool HeightSampler::operator () (const FixedArray<double, 2>& pos, double& z) const {
    FixedArray<double, 2> p = normalization_matrix_.transform(pos);
    return image_(
		(1 - p(1)) * double(image_.original_shape(0) - 1),
		p(0) * double(image_.original_shape(1) - 1),
		z);
}
