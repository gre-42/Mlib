#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Images/StbImage.hpp>
#include <Mlib/Images/Total_Variation/Edge_Image_Config.hpp>
#include <Mlib/Images/Total_Variation/Huber_Rof.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Mapping.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::HuberRof;


int main(int argc, char **argv) {
    enable_floating_point_exceptions();

    ArgParser parser(
        "Usage: sfm_dense_mapping_g"
        " --im <image.png>"
        " --g_out <g_out.png>"
        " [--smoothed_out <smoothed_out.png>]"
        " --alpha <alpha (e.g. 100)>"
        " --beta <beta (e.g. 1.6)>"
        " [--niterations <n>]"
        " [--theta <theta (e.g. 0.2)>]"
        " [--epsilon <epsilon (e.g. 1e-4)>]"
        " [--remove_edge_blobs]",
        {"--remove_edge_blobs"},
        {"--im",
        "--alpha",
        "--beta",
        "--g_out",
        "--smoothed_out",
        "--niterations",
        "--theta",
        "--epsilon"});

    try {
        auto args = parser.parsed(argc, argv);

        StbImage im = StbImage::load_from_file(args.named_value("--im"));

        Array<float> g = g_from_grayscale(
            im.to_float_grayscale(),
            EdgeImageConfig{
                .alpha = safe_stof(args.named_value("--alpha")),
                .beta = safe_stof(args.named_value("--beta")),
                .remove_edge_blobs = args.has_named("--remove_edge_blobs")});

        if (args.has_named_value("--g_out")) {
            StbImage::from_float_grayscale(g).save_to_file(args.named_value("--g_out"));
        }

        if (args.has_named_value("--niterations")) {
            size_t niterations = safe_stoz(args.named_value("--niterations"));
            HuberRofSolver solver{ g };
            solver.a_ = im.to_float_grayscale();
            solver.d_.ref() = solver.a_;
            solver.initialize_q();
            HuberRofConfig huber_rof_config{
                .theta = safe_stof(args.named_value("--theta")),
                .epsilon = safe_stof(args.named_value("--epsilon", "1e-4")),
                .d_min = 0.f,
                .d_max = 1.f};
            for (size_t i = 0; i < niterations; ++i) {
                solver.iterate(huber_rof_config);
            }
            StbImage::from_float_grayscale(solver.d_).save_to_file(args.named_value("--smoothed_out"));
        }

        return 0;
    } catch (const CommandLineArgumentError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
