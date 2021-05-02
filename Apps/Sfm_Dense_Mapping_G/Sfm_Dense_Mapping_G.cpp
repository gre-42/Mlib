#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Sfm/Disparity/Dense_Mapping.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;


int main(int argc, char **argv) {
    enable_floating_point_exceptions();

    ArgParser parser(
        "Usage: sfm_dense --im image.ppm> --dest <dest.ppm> --alpha <alpha> --beta <beta>",
        {},
        {"--im", "--alpha", "--beta", "--dest"});

    try {
        auto args = parser.parsed(argc, argv);

        PpmImage im = PpmImage::load_from_file(args.named_value("--im"));

        Dm::DtamParameters params;
        params.alpha_G_ = safe_stod(args.named_value("--alpha"));
        params.beta_G_ = safe_stod(args.named_value("--beta"));
        PpmImage::from_float_grayscale(Dm::g_from_grayscale(
            im.to_float_grayscale(),
            params)).save_to_file(args.named_value("--dest"));

        return 0;
    } catch (const CommandLineArgumentError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
