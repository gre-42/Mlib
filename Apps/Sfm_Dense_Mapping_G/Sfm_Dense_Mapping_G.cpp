#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Images/StbImage.hpp>
#include <Mlib/Images/Total_Variation/Edge_Image_Config.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Mapping.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::HuberRof;


int main(int argc, char **argv) {
    enable_floating_point_exceptions();

    ArgParser parser(
        "Usage: sfm_dense_mapping_g --im <image.png> --dest <dest.png> --alpha <alpha> --beta <beta> [--remove_edge_blobs]",
        {"--remove_edge_blobs"},
        {"--im", "--alpha", "--beta", "--dest"});

    try {
        auto args = parser.parsed(argc, argv);

        StbImage im = StbImage::load_from_file(args.named_value("--im"));

        StbImage::from_float_grayscale(g_from_grayscale(
            im.to_float_grayscale(),
            EdgeImageConfig{
                .alpha = safe_stof(args.named_value("--alpha")),
                .beta = safe_stof(args.named_value("--beta")),
                .remove_edge_blobs = args.has_named("--remove_edge_blobs")}))
            .save_to_file(args.named_value("--dest"));

        return 0;
    } catch (const CommandLineArgumentError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
