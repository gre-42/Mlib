#pragma once
#include <Mlib/Geometry/Coordinates/Coordinate_Conversion.hpp>
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

namespace Mlib {

template <class TData>
TransformationMatrix<TData, TData, 3> cv_lookat_relative(
    const FixedArray<TData, 3>& camera_pos,
    const FixedArray<TData, 3>& dz,
    const FixedArray<TData, 3>& dy0 = { 0.f, 1.f, 0.f })
{
    auto R = gl_lookat_relative(
        cv_to_opengl_coordinates(dz),
        cv_to_opengl_coordinates(dy0));
    if (!R.has_value()) {
        THROW_OR_ABORT("Could not compute lookat-matrix");
    }
    return opengl_to_cv_extrinsic_matrix(
        TransformationMatrix<float, float, 3>{
            *R,
            cv_to_opengl_coordinates(camera_pos) });
}

}
