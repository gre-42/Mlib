#include <Mlib/Images/Compression/Assemble_Tiles.hpp>
#include <Mlib/Images/Compression/Tile_Image_File.hpp>
#include <Mlib/Images/StbImage1.hpp>
#include <Mlib/Images/StbImage2.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Images/StbImage4.hpp>
#include <Mlib/Images/Transform/Coefficient_Image.hpp>
#include <Mlib/Io/Arg_Parser.hpp>
#include <Mlib/Misc/Floating_Point_Exceptions.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>

using namespace Mlib;

int main(int argc, char** argv) {
    enable_floating_point_exceptions();

    const char* help =
        "Usage: tile_image "
        "[--help] "
        "--json <value> "
        "--width <value> "
        "--height <value> "
        "--channels <value> "
        "--template <filename> "
        "--destination <filename> "
        "--stepsize <value> "
        "--randsize <value> "
        "--alpha <value> "
        "--alpha_fac <value> "
        "--upsampling <value> "
        "[--coeffs] "
        "[--add] "
        "[--ols] ";
    const ArgParser parser(
        help,
        {"--help",
         "--coeffs",
         "--add",
         "--ols"},
        {"--json",
         "--width",
         "--height",
         "--channels",
         "--template",
         "--destination",
         "--alpha",
         "--alpha_fac",
         "--upsampling",
         "--stepsize",
         "--randsize"});
    try {
        const auto args = parser.parsed(argc, argv);
        if (args.has_named("--help")) {
            lout() << help;
            return 0;
        }
        args.assert_num_unnamed(0);
        auto to_opt = [](const Mlib::u8string* s) -> FPath {
            if (s == nullptr) {
                return {};
            }
            return FPath::from_local_path(*s);
        };
        auto fa = [&](){
            if (auto f = args.try_named_value("--json")) {
                return load_fragment_assembly(*f);
            } else {
                return FragmentAssembly{
                    .color = FPath{args.named_value("--template")},
                    .alpha = to_opt(args.try_named_value("--alpha")),
                    .alpha_fac = to_opt(args.try_named_value("--alpha_fac")),
                    .size = {safe_sto<uint32_t>(args.named_svalue("--width")), safe_sto<uint32_t>(args.named_svalue("--height"))},
                    .stepsize = safe_stof(args.named_svalue("--stepsize")),
                    .randsize = safe_stof(args.named_svalue("--randsize")),
                    .channels = safe_sto<uint32_t>(args.named_svalue("--channels")),
                    .add = args.has_named("--add"),
                    .upsampling = safe_sto<uint32_t>(args.named_svalue("--upsampling", "1")),
                    .ols = args.has_named("--ols")
                        ? std::vector<OlsCoefficient>()
                        : std::optional<std::vector<OlsCoefficient>>{}
                };
            }
        }();
        Array<float> down;
        if (args.has_named("--coeffs")) {
            CoefficientImageCache coeffs;
            assemble_tiles_compute_ols(fa, &coeffs);
            down = assemble_tiles_compute_ols(fa, &coeffs);
        } else {
            down = assemble_tiles_compute_ols(fa, nullptr);
        }
        switch (fa.channels - size_t(fa.add)) {
        case 1:
            StbImage1::from_float_grayscale(down).save_to_file(args.named_value("--destination"));
            return 0;
        case 2:
            StbImage2::from_float_ia(down).save_to_file(args.named_value("--destination"));
            return 0;
        case 3:
            StbImage3::from_float_rgb(down).save_to_file(args.named_value("--destination"));
            return 0;
        case 4:
            StbImage4::from_float_rgba(down).save_to_file(args.named_value("--destination"));
            return 0;
        }
        throw std::runtime_error("Unexpected number of channels");
    } catch (const std::exception& e) {
        lerr() << e.what();
        if (unhandled_exceptions_occured()) {
            print_unhandled_exceptions();
        }
        return 1;
    }
    return 0;
}
