#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Cv/Depth_Difference.hpp>
#include <Mlib/Cv/Depth_Map_Package.hpp>
#include <Mlib/Cv/Render_Data.hpp>
#include <Mlib/Images/Filters/Median_Filter.hpp>
#include <Mlib/Images/StbImage.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <vector>

using namespace Mlib;
using namespace Mlib::Cv;

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: render_depth_map"
        " --rgb <filename>"
        " --depth <filename>"
        " --ki <intrinsic_matrix>"
        " [--ke <extrinsic_matrix>]"
        " [--median_filter_radius <r>]"
        " [--z_offset <z_offset>]"
        " [--rotate]"
        " [--minus <file1> <file2...>]",
        {"--rotate"},
        {"--rgb",
        "--depth",
        "--ki",
        "--ke",
        "--z_offset",
        "--median_filter_radius",
        "--near_plane",
        "--far_plane",
        "--minus_threshold"},
        {"--minus"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unamed(0);

        auto load_depth = [&args](const std::string& filename){
            Array<float> depth = Array<float>::load_binary(filename);
            size_t r = safe_stoz(args.named_value("--median_filter_radius", "0"));
            if (r != 0) {
                depth = median_filter_2d(depth, r);
            }
            return depth;
        };
        auto load_ke = [](const std::string& filename){
            Array<float> ke = Array<float>::load_txt_2d(filename);
            if (!all(ke.shape() == ArrayShape{4, 4})) {
                throw std::runtime_error("Extrinsic matrix has incorrect shape");
            }
            return TransformationMatrix<float, 3>{ FixedArray<float, 4, 4>{ ke }};
        };
        auto load_ki = [](const std::string& filename){
            Array<float> ki = Array<float>::load_txt_2d(filename);
            if (!all(ki.shape() == ArrayShape{3, 3})) {
                throw std::runtime_error("Intrinsic matrix has incorrect shape");
            }
            return TransformationMatrix<float, 2>{ FixedArray<float, 3, 3>{ ki }};
        };

        StbImage img = StbImage::load_from_file(args.named_value("--rgb"));
        Array<float> depth = load_depth(args.named_value("--depth"));
        TransformationMatrix<float, 2> intrinsic_matrix = load_ki(args.named_value("--ki"));
        if (!all(depth.shape() == img.shape())) {
            throw std::runtime_error("Depth and image shape differ");
        }
        if (args.has_named_list("--minus")) {
            for (const std::string& filename : args.named_list("--minus")) {
                DepthMapPackage pkg = load_depth_map_package(filename);
                TransformationMatrix<float, 3> ke0 = load_ke(args.named_value("--ke"));
                Array<float> diff = Cv::depth_difference(depth, pkg.depth, intrinsic_matrix, pkg.ki, projection_in_reference(ke0, pkg.ke));
                float thresh = safe_stof(args.named_value("--minus_threshold"));
                depth = depth.array_array_binop(diff, [&thresh](float a, float b){ return std::isnan(b) || (b < -thresh) ? NAN : a; });
            }
        }
        size_t num_renderings = SIZE_MAX;
        RenderConfig render_config{
            .screen_width = (int)depth.shape(1),
            .screen_height = (int)depth.shape(0)};
        SceneNodeResources scene_node_resources;
        RenderingContextGuard rrg{scene_node_resources, "primary_rendering_resources", render_config.anisotropic_filtering_level, 0};
        Render2 render{ render_config, num_renderings };
        render_depth_map(
            render,
            img.to_float_rgb(),
            depth,
            intrinsic_matrix,
            safe_stof(args.named_value("--near_plane", "0.1")),
            safe_stof(args.named_value("--far_plane", "100")),
            safe_stof(args.named_value("--z_offset", "1")),
            args.has_named("--rotate"));
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
