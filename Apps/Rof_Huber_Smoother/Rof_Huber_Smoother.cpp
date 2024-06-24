#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Cv/Depth_Map_Package.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Images/Total_Variation/Edge_Image_Config.hpp>
#include <Mlib/Images/Total_Variation/Huber_Rof.hpp>
#include <Mlib/Sfm/Disparity/Dsi/Inverse_Depth_Cost_Volume.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Mapping.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::HuberRof;


int main(int argc, char **argv) {
    enable_floating_point_exceptions();

    ArgParser parser(
        "Usage: sfm_dense_mapping_g"
        " --im <image.png>"
        " [--dsi <dsi.array>]"
        " --g_out <g_out.png>"
        " [--smoothed_out <smoothed_out.png>]"
        " [--depth_out <depth_out.png>]"
        " [--depth_array_out <depth_array_out.array>]"
        " [--package_out <package_out.png>]"
        " [--intrinsic_matrix <intrinsic_matrix.m>]"
        " [--extrinsic_matrix <extrinsic_matrix.m>]"
        " [--true_depth <true_depth.array>]"
        " --alpha <alpha (e.g. 100)>"
        " --beta_G <beta (e.g. 1.6)>"
        " --lambda <lambda (e.g. 1.0)>"
        " --beta_I <beta (e.g. 0.0001 or 0.001)>"
        " [--niterations <n>]"
        " [--theta_0 <theta_0 (e.g. 0.2)>]"
        " [--theta_end <theta_end (e.g. 1e-4)>]"
        " [--epsilon <epsilon (e.g. 1e-4)>]"
        " [--remove_edge_blobs]"
        " [--optimize_with_true_depth]"
        " [--draw_lm_pngs]"
        " [--optimize_parameters]"
        " [--min_depth]"
        " [--max_depth]",
        {"--remove_edge_blobs",
        "--optimize_with_true_depth",
        "--draw_lm_pngs",
        "--optimize_parameters"},
        {"--im",
        "--dsi",
        "--lambda",
        "--beta_I",
        "--alpha",
        "--beta_G",
        "--g_out",
        "--smoothed_out",
        "--niterations",
        "--theta_0",
        "--theta_end",
        "--epsilon",
        "--depth_out",
        "--depth_array_out",
        "--package_out",
        "--intrinsic_matrix",
        "--extrinsic_matrix",
        "--true_depth",
        "--min_depth",
        "--max_depth"});

    try {
        auto args = parser.parsed(argc, argv);

        StbImage3 im = StbImage3::load_from_file(args.named_value("--im"));
        (Array<Rgb24>&)im = Rgb24::white();

        EdgeImageConfig edge_image_config{
            .alpha = safe_stof(args.named_value("--alpha")),
            .beta = safe_stof(args.named_value("--beta_G")),
            .remove_edge_blobs = args.has_named("--remove_edge_blobs")};

        Array<float> g = g_from_grayscale(
            im.to_float_grayscale(),
            edge_image_config);

        if (args.has_named_value("--g_out")) {
            StbImage3::from_float_grayscale(0.1f * g).save_to_file(args.named_value("--g_out"));
        }

        if (args.has_named_value("--depth_out") ||
            args.has_named_value("--depth_array_out") ||
            args.has_named_value("--package_out") ||
            args.has_named("--optimize_with_true_depth") ||
            args.has_named("--optimize_parameters"))
        {
            Array<float> dsi = Array<float>::load_binary(args.named_value("--dsi"));
            if (dsi.ndim() != 3) {
                throw std::runtime_error("DSI has incorrect number of dimensions");
            }
            if (!all(dsi.shape().erased_first() == g.shape())) {
                throw std::runtime_error("DSI has incorrect shape");
            }
            Dm::DtamParameters params(
                edge_image_config,
                safe_stof(args.named_value("--theta_0")),                    // theta_0
                safe_stof(args.named_value("--theta_end")),                  // theta_end
                safe_stof(args.named_value("--beta_I")),                     // beta
                safe_stof(args.named_value("--lambda")),
                safe_stof(args.named_value("--epsilon")),
                safe_stoz(args.named_value("--niterations")));
            CostVolumeParameters cost_volume_parameters{
                .min_depth = safe_stof(args.named_value("--min_depth")),
                .max_depth = safe_stof(args.named_value("--max_depth")),
                .ndepths = dsi.shape(0)};
            if (args.has_named("--optimize_with_true_depth")) {
                Array<float> true_ai = 1.f / Array<float>::load_binary(args.named_value("--true_depth"));
                if (!all(true_ai.shape() == g.shape())) {
                    throw std::runtime_error("True depth shape incorrect");
                }
                Dm::quantitative_primary_parameter_optimization_lm(
                    dsi,
                    im.to_float_grayscale(),
                    true_ai,
                    cost_volume_parameters,
                    params,
                    args.has_named("--draw_lm_pngs"));  // draw_bmps
            }
            if (args.has_named("--optimize_parameters")) {
                Dm::qualitative_primary_parameter_optimization(dsi, g, cost_volume_parameters, params);
                Dm::auxiliary_parameter_optimization(dsi, g, cost_volume_parameters, params);
            }
            if (args.has_named_value("--depth_out") ||
                args.has_named_value("--depth_array_out") ||
                args.has_named_value("--package_out"))
            {
                Dm::DenseMapping dm{
                    g,
                    cost_volume_parameters,
                    params,
                    false,                              // print_energy
                    false};                             // print_bmps
                dm.notify_cost_volume_changed(InverseDepthCostVolume{ dsi });
                dm.iterate_atmost(SIZE_MAX);
                Array<float> ai = dm.interpolated_inverse_depth_image();
                if (args.has_named_value("--depth_out")) {
                    draw_nan_masked_grayscale(
                        ai,
                        1.f / cost_volume_parameters.max_depth,
                        1.f / cost_volume_parameters.min_depth)
                    .save_to_file(args.named_value("--depth_out"));
                }
                if (args.has_named_value("--depth_array_out")) {
                    (1.f / ai).save_binary(args.named_value("--depth_array_out"));
                }
                if (args.has_named_value("--package_out")) {
                    Cv::save_depth_map_package(
                        args.named_value("--package_out"),
                        std::chrono::milliseconds{0},
                        args.named_value("--im"),
                        args.named_value("--depth_array_out"),
                        args.named_value("--intrinsic_matrix"),
                        args.named_value("--extrinsic_matrix"));
                }
            }
        }
        if (args.has_named_value("--smoothed_out")) {
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
            StbImage3::from_float_grayscale(solver.d_).save_to_file(args.named_value("--smoothed_out"));
        }

        return 0;
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
}
