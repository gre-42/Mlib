#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Coordinates/Coordinate_Conversion.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <iostream>

using namespace Mlib;

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: intrinsic_matrix_from_dimensions --focal_length <f> --sensor_size_x <x> --sensor_size_y <y> --picture_rows <rows> --picture_cols <cols>",
        {},
        {"--focal_length", "--sensor_size_x", "--sensor_size_y", "--picture_rows", "--picture_cols"});

    const auto args = parser.parsed(argc, argv);

    FixedArray<float, 2> sensor_size{
        safe_stof(args.named_value("--sensor_size_x")),
        safe_stof(args.named_value("--sensor_size_y"))};

    FixedArray<size_t, 2> image_shape{
        safe_stoz(args.named_value("--picture_rows")),
        safe_stoz(args.named_value("--picture_cols"))};

    std::cout << intrinsic_matrix_from_dimensions(
        safe_stof(args.named_value("--focal_length")),
        sensor_size,
        image_shape).affine() << std::endl;

    return 0;
}
