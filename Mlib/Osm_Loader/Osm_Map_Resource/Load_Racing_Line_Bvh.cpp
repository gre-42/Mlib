#include "Load_Racing_Line_Bvh.hpp"
#include <Mlib/Array/Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Racing_Line_Bvh.hpp>

using namespace Mlib;

void Mlib::load_racing_line_bvh(
    const std::string& filename,
    TransformationMatrix<double, 2>& normalization_matrix,
    RacingLineBvh& racing_line_bvh)
{
    auto mat = Array<double>::load_txt_2d(filename, ArrayShape{0, 2});
    if (mat.shape(1) != 2) {
        throw std::runtime_error("File \"" + filename + "\" does not have 2 columns");
    }
    for (size_t r = 1; r < mat.shape(0); ++r) {
        racing_line_bvh.insert(
            FixedArray<FixedArray<float, 2>, 2>{
                normalization_matrix.transform(FixedArray<double, 2>{ mat(r - 1, 0), mat(r - 1, 1) }).casted<float>(),
                normalization_matrix.transform(FixedArray<double, 2>{ mat(r, 0), mat(r, 1) }).casted<float>()});
    }
}
