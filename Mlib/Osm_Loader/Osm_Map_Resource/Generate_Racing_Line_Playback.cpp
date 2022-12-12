#include "Generate_Racing_Line_Playback.hpp"
#include <Mlib/Array/Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Ground_Bvh.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static const size_t LAT = 0;
static const size_t LON = 1;
static const size_t YANGLE = 2;
static const size_t TIME = 3;
static const size_t ACCEL = 4;
static const size_t BREAK = 5;

void Mlib::generate_racing_line_playback(
    const std::string& racing_line_filename,
    const std::string& playback_filename,
    const TransformationMatrix<double, double, 2>& normalization_matrix,
    const TransformationMatrix<double, double, 3>& geographic_mapping,
    const GroundBvh& ground_bvh)
{
    auto mat = Array<double>::load_txt_2d(racing_line_filename, ArrayShape{0, 6});
    if (mat.shape(1) != 6) {
        throw std::runtime_error("File \"" + racing_line_filename + "\" does not have 6 columns");
    }
    std::ofstream ofstr{playback_filename};
    if (ofstr.fail()) {
        THROW_OR_ABORT("Could not open racing line file \"" + playback_filename + "\" for writing");
    }
    ofstr << std::setprecision(18) << std::scientific;
    for (const auto& row : mat) {
        auto pos = normalization_matrix.transform(FixedArray<double, 2>{ row(LAT), row(LON) });
        double height;
        if (!ground_bvh.height(height, pos)) {
            throw std::runtime_error("Could not find height for point on racing line");
        }
        auto xpos = geographic_mapping.transform(FixedArray<double, 3>{pos(0), pos(1), height});
        // ofstr << xpos << ' ' << row(YANGLE) << ' ' << row(TIME) << ' ' << row(ACCEL) << ' ' << row(BREAK) << '\n';
        ofstr << row(TIME) << ' ' << xpos << " 0 " << row(YANGLE) << " 0\n";
    }
    ofstr.flush();
    if (ofstr.fail()) {
        THROW_OR_ABORT("Could not write to file \"" + playback_filename + '"');
    }
}
