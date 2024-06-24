#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Images/Extrapolate_Rgba_Colors.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Images/StbImage4.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

int main(int argc, char** argv) {
    enable_floating_point_exceptions();
    const ArgParser parser(
        "Usage: extrapolate_alpha_texture "
        "source "
        "destination "
        "[--remove_alpha] "
        "--sigma <sigma> "
        "--niterations <niterations> "
        "--debug <debug>",
        {"--remove_alpha"},
        {"--sigma",
         "--niterations",
         "--debug"});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed(2);
        auto xp = extrapolate_rgba_colors(
            StbImage4::load_from_file(args.unnamed_value(0)),
            safe_stof(args.named_value("--sigma")),
            safe_stoz(args.named_value("--niterations")));
        if (args.has_named("--remove_alpha")) {
            xp.to_rgb().save_to_file(args.unnamed_value(1));
        } else {
            xp.save_to_file(args.unnamed_value(1));
        }

        if (args.has_named_value("--debug")) {
            auto dest = StbImage4::load_from_file(args.unnamed_value(1)).to_float_rgba();
            StbImage3::from_float_rgb(dest.row_range(0, 3)).save_to_file(args.named_value("--debug"));
        }
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
