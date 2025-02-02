#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Bgr565Bitmap.hpp>
#include <Mlib/Images/Filters/Box_Filter.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <iostream>

using namespace Mlib;

void box_filter_file(
    const std::string& source,
    const std::string& destination,
    size_t width,
    size_t niter)
{
    auto bitmap = Bgr565Bitmap::load_from_file(source);

    Array<float> image = bitmap.to_float_grayscale();
    for (size_t i = 0; i < niter; ++i) {
        image = box_filter_NWE(image, ArrayShape{ width, width });
    }

    Bgr565Bitmap::from_float_grayscale(image).save_to_file(destination);
}

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: box_filter source destination --width <width> --niter <niter>",
        {},
        {"--width", "--niter"});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unnamed(2);
    box_filter_file(
        args.unnamed_value(0),
        args.unnamed_value(1),
        safe_stoz(args.named_value("--width")),
        safe_stoz(args.named_value("--niter")));
    return 0;
}
