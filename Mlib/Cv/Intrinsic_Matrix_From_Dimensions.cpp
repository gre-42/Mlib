#include "Intrinsic_Matrix_From_Dimensions.hpp"
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Coordinates.hpp>


using namespace Mlib;
using namespace Mlib::Cv;

Array<float> Mlib::Cv::intrinsic_matrix_from_dimensions(
    float focal_length,
    const Array<float>& sensor_size,
    const ArrayShape& picture_shape)
{
    assert(sensor_size.length() == 2);
    assert(picture_shape.ndim() == 2);
    return Array<float>{
        {focal_length / sensor_size(0) * (picture_shape(id1) - 1), 0, picture_shape(id1) / 2.f},
        {0, focal_length / sensor_size(1) * (picture_shape(id0) - 1), picture_shape(id0) / 2.f},
        {0, 0, 1}};
}
