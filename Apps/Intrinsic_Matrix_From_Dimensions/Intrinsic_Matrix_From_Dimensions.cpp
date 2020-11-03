#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Array/Array.hpp>
#include <Mlib/Cv/Intrinsic_Matrix_From_Dimensions.hpp>
#include <Mlib/String.hpp>
#include <iostream>

using namespace Mlib;
using namespace Mlib::Cv;

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: intrinsic_matrix_from_dimensions --focal_length <f> --sensor_size_x <x> --sensor_size_y <y> --picture_rows <rows> --picture_cols <cols>",
        {},
        {"--focal_length", "--sensor_size_x", "--sensor_size_y", "--picture_rows", "--picture_cols"});

    const auto args = parser.parsed(argc, argv);

    Array<float> sensor_size{
        (float)safe_stod(args.named_value("--sensor_size_x")),
        (float)safe_stod(args.named_value("--sensor_size_y"))};

    ArrayShape image_shape{
        safe_stoz(args.named_value("--picture_rows")),
        safe_stoz(args.named_value("--picture_cols"))};

    std::cout << intrinsic_matrix_from_dimensions(
        safe_stoi(args.named_value("--focal_length")),
        sensor_size,
        image_shape) << std::endl;

    return 0;
}
