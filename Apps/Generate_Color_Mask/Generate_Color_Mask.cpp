#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Images/StbImage1.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <stb_cpp/stb_generate_color_mask.hpp>
#include <stb_cpp/stb_transform.hpp>

using namespace Mlib;

int main(int argc, char** argv) {
    enable_floating_point_exceptions();
    const ArgParser parser(
        "Usage: generate_color_mask source destination -r <r> -g <g> -b <b> --near <near> --far <far> [--invert]",
        {"--invert"},
        {"-r",
         "-g",
         "-b",
        "--near",
        "--far"});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed(2);
        auto im_in = StbImage3::load_from_file(args.unnamed_value(0));
        auto im_out = StbImage1{ im_in.shape() };
        short color[3] = {
            safe_stox<short>(args.named_value("-r")),
            safe_stox<short>(args.named_value("-g")),
            safe_stox<short>(args.named_value("-b"))};
        stb_generate_color_mask(
            &im_in.flat_begin()->r,
            im_out.flat_begin(),
            integral_cast<int>(im_in.shape(1)),
            integral_cast<int>(im_in.shape(0)),
            3,
            color,
            safe_stox<unsigned short>(args.named_value("--near")),
            safe_stox<unsigned short>(args.named_value("--far")));
        if (args.has_named("--invert")) {
            stb_transform(
                im_out.flat_begin(),
                integral_cast<int>(im_out.shape(1)),
                integral_cast<int>(im_out.shape(0)),
                1,      // nchannels
                -1.f,   // times
                1.f,    // plus
                false); // abs
        }
        im_out.save_to_file(args.unnamed_value(1));
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
