#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Cv/Depth_Map_Package.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Sfm/Disparity/Dsi/Inverse_Depth_Cost_Volume.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Geometry.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;


int main(int argc, char **argv) {
    enable_floating_point_exceptions();

    ArgParser parser(
        "Usage: sfm_dense_mapping_g"
        " [--dsi <dsi.array>]"
        " [--depth_out <depth_out.png>]"
        " [--depth_array_out <depth_array_out.array>]"
        " [--package_out <package_out.png>]"
        " [--intrinsic_matrix <intrinsic_matrix.m>]"
        " [--extrinsic_matrix <extrinsic_matrix.m>]"
        " --lambda <lambda (e.g. 1.0)>"
        " --beta <beta (e.g. 0.0001 or 0.001)>"
        " [--niterations <n>]"
        " [--niterations_inner <n>]"
        " [--theta_0 <theta_0 (e.g. 0.2)>]"
        " [--theta_end <theta_end (e.g. 1e-4)>]"
        " [--optimize_parameters]"
        " [--min_depth]"
        " [--max_depth]",
        {"--optimize_parameters"},
        {"--dsi",
        "--lambda",
        "--beta",
        "--niterations",
        "--niterations_inner",
        "--theta_0",
        "--theta_end",
        "--tau",
        "--depth_out",
        "--depth_array_out",
        "--package_out",
        "--intrinsic_matrix",
        "--extrinsic_matrix",
        "--min_depth",
        "--max_depth"});

    try {
        auto args = parser.parsed(argc, argv);

        Array<float> dsi = Array<float>::load_binary(args.named_value("--dsi"));
        if (dsi.ndim() != 3) {
            throw std::runtime_error("DSI has incorrect number of dimensions");
        }
        Dg::DenseGeometryParameters params{
            .theta_0__ = safe_stof(args.named_value("--theta_0")),
            .theta_end__ = safe_stof(args.named_value("--theta_end")),
            .beta = safe_stof(args.named_value("--beta")),
            .lambda__ = safe_stof(args.named_value("--lambda")),
            .tau = safe_stof(args.named_value("--tau", "0.125")),
            .nsteps = safe_stoz(args.named_value("--niterations")),
            .nsteps_inner = safe_stoz(args.named_value("--niterations_inner", "1"))
        };
        CostVolumeParameters cost_volume_parameters{
            .min_depth = safe_stof(args.named_value("--min_depth")),
            .max_depth = safe_stof(args.named_value("--max_depth")),
            .ndepths = dsi.shape(0)};
        if (args.has_named("--optimize_parameters")) {
            Dg::qualitative_primary_parameter_optimization(dsi, cost_volume_parameters, params);
            Dg::auxiliary_parameter_optimization(dsi, cost_volume_parameters, params);
        }
        if (args.has_named_value("--depth_out") ||
            args.has_named_value("--depth_array_out") ||
            args.has_named_value("--package_out"))
        {
            Dg::DenseGeometry dg{
                cost_volume_parameters,
                params,
                false,                              // print_energy
                false};                             // print_bmps
            dg.notify_cost_volume_changed(InverseDepthCostVolume{ dsi });
            dg.iterate_atmost(SIZE_MAX);
            Array<float> ai = dg.interpolated_inverse_depth_image();
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

        return 0;
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
}
