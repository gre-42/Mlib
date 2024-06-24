#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Make_Seamless.hpp>
#include <Mlib/Images/StbImage1.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: make_seamless"
        " --src <src>"
        " --dest <dest>"
        " --overlap <overlap>"
        " [--grayscale]",
        {"--grayscale"},
        {"--src",
         "--dest",
         "--overlap"});

    try {
        const auto args = parser.parsed(argc, argv);

        if (args.has_named("--grayscale")) {
            auto dest = make_symmetric_2d(
                StbImage1::load_from_file(args.named_value("--src")).to_float_grayscale(),
                safe_stoz(args.named_value("--overlap")));

            StbImage1::from_float_grayscale(dest).save_to_file(args.named_value("--dest"));
        } else {
            auto dest = make_symmetric_2d_multichannel(
                StbImage3::load_from_file(args.named_value("--src")).to_float_rgb(),
                safe_stoz(args.named_value("--overlap")));

            StbImage3::from_float_rgb(dest).save_to_file(args.named_value("--dest"));
        }
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
