#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Cv/Depth_Map_Package.hpp>
#include <Mlib/Cv/Depth_Minus.hpp>
#include <Mlib/Cv/Render/Render_Data.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Images/Filters/Median_Filter.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Render/Input_Config.hpp>
#include <Mlib/Render/Render.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Particle_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Trail_Resources.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>
#include <vector>

using namespace Mlib;
using namespace Mlib::Cv;

int main(int argc, char** argv) {
    enable_floating_point_exceptions();

    const ArgParser parser(
        "Usage: render_depth_map"
        " --rgb <filename>"
        " --depth <filename>"
        " --ki <intrinsic_matrix>"
        " [--ke <extrinsic_matrix>]"
        " [--median_filter_radius <r>]"
        " [--rotate]"
        " [--minus <file1> <file2...>]",
        {"--rotate"},
        {"--rgb",
        "--depth",
        "--ki",
        "--ke",
        "--median_filter_radius",
        "--near_plane",
        "--far_plane",
        "--minus_threshold"},
        {"--minus"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unnamed(0);

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
            return TransformationMatrix<float, float, 3>{ FixedArray<float, 4, 4>{ ke }};
        };
        auto load_ki = [](const std::string& filename){
            Array<float> ki = Array<float>::load_txt_2d(filename);
            if (!all(ki.shape() == ArrayShape{3, 3})) {
                throw std::runtime_error("Intrinsic matrix has incorrect shape");
            }
            return TransformationMatrix<float, float, 2>{ FixedArray<float, 3, 3>{ ki }};
        };

        StbImage3 img = StbImage3::load_from_file(args.named_value("--rgb"));
        Array<float> depth = load_depth(args.named_value("--depth"));
        TransformationMatrix<float, float, 2> intrinsic_matrix = load_ki(args.named_value("--ki"));
        if (!all(depth.shape() == img.shape())) {
            throw std::runtime_error("Depth and image shape differ");
        }
        if (args.has_named_list("--minus")) {
            for (const std::string& filename : args.named_list("--minus")) {
                DepthMapPackage pkg = load_depth_map_package(filename);
                TransformationMatrix<float, float, 3> ke0 = load_ke(args.named_value("--ke"));
                Array<float> diff = Cv::depth_minus(depth, pkg.depth, intrinsic_matrix, pkg.ki, projection_in_reference(ke0, pkg.ke));
                float thresh = safe_stof(args.named_value("--minus_threshold"));
                depth = depth.array_array_binop(diff, [&thresh](float a, float b){ return std::isnan(b) || (b < -thresh) ? NAN : a; });
            }
        }
        std::atomic_size_t num_renderings = SIZE_MAX;
        RenderConfig render_config{
            .windowed_width = (int)depth.shape(1),
            .windowed_height = (int)depth.shape(0)};
        InputConfig input_config;
        SceneNodeResources scene_node_resources;
        ParticleResources particle_resources;
        TrailResources trail_resources;
        RenderingResources rendering_resources{
            "primary_rendering_resources",
            16 };
        RenderingContext rendering_context{
            scene_node_resources,
            particle_resources,
            trail_resources,
            rendering_resources };
        RenderingContextGuard rrg{ rendering_context };
        SetFps set_fps{ nullptr };
        Render render{ render_config, input_config, num_renderings, set_fps, []() { return std::chrono::steady_clock::now(); } };
        render_depth_map(
            render,
            img.to_float_rgb(),
            depth,
            intrinsic_matrix,
            safe_stof(args.named_value("--near_plane", "0.1")),
            safe_stof(args.named_value("--far_plane", "100")),
            args.has_named("--rotate"));
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
