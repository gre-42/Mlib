#include "Load_Racing_Line_Bvh.hpp"
#include <Mlib/Array/Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Racing_Line_Bvh.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static const size_t LAT = 0;
static const size_t LON = 1;
// static const size_t YANGLE = 2;
// static const size_t TIME = 3;
static const size_t F_DRIVE = 4;
static const size_t F_BREAK = 5;

static const float F_DRIVE_HIGH = 5000.f;
static const float F_BREAK_HIGH = 5000.f;


void Mlib::load_racing_line_bvh(
    const std::string& filename,
    const TransformationMatrix<double, double, 2>& normalization_matrix,
    RacingLineBvh& racing_line_bvh)
{
    auto mat = Array<double>::load_txt_2d(filename, ArrayShape{ 0, 6 });
    if (mat.shape(1) != 6) {
        THROW_OR_ABORT("File \"" + filename + "\" does not have 6 columns");
    }
    for (size_t r = 1; r < mat.shape(0); ++r) {
        if (std::isnan(mat(r - 1, F_DRIVE)) || std::isnan(mat(r, F_DRIVE)) ||
            std::isnan(mat(r - 1, F_BREAK)) || std::isnan(mat(r, F_BREAK)))
        {
            continue;
        }
        float c_drive = std::max(0.f, (float)mat(r - 1, F_DRIVE) / F_DRIVE_HIGH);
        float c_break = std::max(0.f, -(float)mat(r - 1, F_BREAK) / F_BREAK_HIGH);
        float c_idle = std::max(0.f, 1.f - c_drive - c_break);
        racing_line_bvh.insert(
            RacingLineSegment{
                .racing_line_segment = FixedArray<ScenePos, 2, 2>{
                    normalization_matrix.transform(FixedArray<ScenePos, 2>{ mat(r - 1, LAT), mat(r - 1, LON) }),
                    normalization_matrix.transform(FixedArray<ScenePos, 2>{ mat(r, LAT), mat(r, LON) })}
                    .casted<CompressedScenePos>(),
                .color = FixedArray<float, 3>{c_break, c_idle, c_drive}});
    }
}
