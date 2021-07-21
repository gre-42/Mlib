#include "Intrinsic_Matrix_From_Dimensions.hpp"
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>

using namespace Mlib;
using namespace Mlib::Cv;

TransformationMatrix<float, 2> Mlib::Cv::intrinsic_matrix_from_dimensions(
    float focal_length,
    const FixedArray<float, 2>& sensor_size,
    const FixedArray<size_t, 2>& picture_shape)
{
    return TransformationMatrix<float, 2>{
        FixedArray<float, 2, 2>{
            focal_length / sensor_size(0) * (picture_shape(id1) - 1), 0,
            0, focal_length / sensor_size(1) * (picture_shape(id0) - 1)},
        // 0.5f + (picture_shape(id1) - 1) / 2.f
        // = picture_shape(id1) / 2.f
        FixedArray<float, 2>{
            picture_shape(id1) / 2.f,
            picture_shape(id0) / 2.f}};
}
