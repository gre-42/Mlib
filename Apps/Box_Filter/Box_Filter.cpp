#include <Mlib/Images/Filters/Box_Filter.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Io/Arg_Parser.hpp>
#include <Mlib/Os/Utf8_Path.hpp>
#include <Mlib/Strings/String_View_To_Number.hpp>
#include <iostream>

using namespace Mlib;

void box_filter_file(
    const Utf8Path& source,
    const Utf8Path& destination,
    size_t width,
    size_t niter)
{
    auto bitmap = StbImage3::load_from_file(source);

    Array<float> image = bitmap.to_float_grayscale();
    for (size_t i = 0; i < niter; ++i) {
        image = box_filter_NWE(image, ArrayShape{ width, width });
    }

    StbImage3::from_float_grayscale(image).save_to_file(destination);
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
        safe_stoz(args.named_svalue("--width")),
        safe_stoz(args.named_svalue("--niter")));
    return 0;
}
