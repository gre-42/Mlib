#pragma once
#include <Mlib/Geometry/Coordinate_Conversion.hpp>
#include <Mlib/Geometry/Gl_Look_At.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>

namespace Mlib {

template <class TData>
TransformationMatrix<TData, 3> cv_lookat_relative(
    const FixedArray<TData, 3>& camera_pos,
    const FixedArray<TData, 3>& dz,
    const FixedArray<TData, 3>& dy0 = { 0.f, 1.f, 0.f })
{
    return opengl_to_cv_extrinsic_matrix(
        TransformationMatrix<float, 3>{
            gl_lookat_relative(
                cv_to_opengl_coordinates(dz),
                cv_to_opengl_coordinates(dy0)),
            cv_to_opengl_coordinates(camera_pos) });
}

}
