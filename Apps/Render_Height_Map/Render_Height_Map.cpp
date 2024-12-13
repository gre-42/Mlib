#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geography/Heightmaps/Load_Heightmap_From_File.hpp>
#include <Mlib/Geometry/Coordinates/Normalized_Points_Fixed.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Pgm_Image.hpp>
#include <Mlib/Images/Resample/Pyramid.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Render/Input_Config.hpp>
#include <Mlib/Render/Normal_Type.hpp>
#include <Mlib/Render/Render.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Height_Map.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Particle_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Trail_Resources.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Threads/Realtime_Threads.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <Mlib/Time/Fps/Fixed_Time_Sleeper.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>
#include <stb_cpp/stb_array.hpp>
#include <stb_cpp/stb_image_load.hpp>
#include <vector>

using namespace Mlib;

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: render_height_map [--rgb <filename.png>] --height <filename.{pgm,jpg,png}> [--xy_scale <scale>] [--z_scale <scale>] [--rotate] [--down_sample <n>] [--normal_type {face,vertex}]",
        {"--rotate"},
        {"--rgb", "--height", "--xy_scale", "--z_scale", "--down_sample", "--normal_type"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unnamed(0);

        reserve_realtime_threads(0);

        Array<float> height = load_heightmap_from_file<float>(args.named_value("--height"));
        Array<float> color = args.has_named_value("--rgb")
            ? StbImage3::load_from_file(args.named_value("--rgb")).to_float_rgb()
            : StbImage3{ height.fixed_shape<2>(), Rgb24::white() }.to_float_rgb();
        if (!all(height.shape() == color.shape().erased_first())) {
            throw std::runtime_error("Depth and image shape differ");
        }
        for (size_t i = 0; i < safe_stoz(args.named_value("--down_sample", "0")); ++i) {
            height.move() = down_sample2(height);
            color.move() = multichannel_down_sample2(color);
        }
        NormalizedPointsFixed<float> np{ScaleMode::PRESERVE_ASPECT_RATIO, OffsetMode::CENTERED};
        np.add_point({0.f, 0.f});
        np.add_point({float(color.shape(1 + id1)) - 1, float(color.shape(1 + id0)) - 1});

        std::atomic_size_t num_renderings = SIZE_MAX;
        RenderConfig render_config;
        InputConfig input_config;
        FixedTimeSleeper sleeper{ safe_stof(args.named_value("--sleep_dt", "0.01667")) };
        SetFps set_fps{ &sleeper };
        Render render{
            render_config,
            input_config,
            num_renderings,
            set_fps,
            []() { return std::chrono::steady_clock::now(); }
        };
        SceneNodeResources scene_node_resources;
        ParticleResources particle_resources;
        TrailResources trail_resources;
        RenderingResources rendering_resources{
            "primary_rendering_resources",
            16 };
        RenderingContext primary_rendering_context{
            .scene_node_resources = scene_node_resources,
            .particle_resources = particle_resources,
            .trail_resources = trail_resources,
            .rendering_resources = rendering_resources,
            .z_order = 0 };
        RenderingContextGuard rcg{ primary_rendering_context };
        render_height_map(
            render,
            color,
            height * safe_stof(args.named_value("--z_scale", "0.001")),
            np.normalization_matrix().pre_scaled(safe_stof(args.named_value("--xy_scale", "1"))),
            normal_type_from_string(args.named_value("--normal_type", "face")),
            args.has_named("--rotate"));
        if (unhandled_exceptions_occured()) {
            print_unhandled_exceptions();
            return 1;
        }
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
