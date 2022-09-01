#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Cv/Render/Render_Data.hpp>
#include <Mlib/Geometry/Coordinates/Normalized_Points_Fixed.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/PgmImage.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Render/Cameras/Generic_Camera.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <vector>

using namespace Mlib;
using namespace Mlib::Cv;

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: render_point_cloud --points <filename.m> [--scale <scale>] [--rotate]",
        {"--rotate"},
        {"--points", "--scale"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unamed(0);

        Array<FixedArray<float, 3>> points = Array<float>::from_dynamic<3>(Array<float>::load_txt_2d(args.named_value("--points")));
        SceneNodeResources scene_node_resources;
        RenderingContextGuard rrg{scene_node_resources, "primary_rendering_resources", 16, 0};
        std::atomic_size_t num_renderings = SIZE_MAX;
        RenderConfig render_config;
        std::unique_ptr<Camera> camera(new GenericCamera(
            CameraConfig(),
            GenericCamera::Postprocessing::ENABLED,
            GenericCamera::Mode::PERSPECTIVE));
        Render2 render{ render_config, num_renderings };
        render_point_cloud(
            render,
            points.applied<TransformationMatrix<float, float, 3>>([](const auto& p){return TransformationMatrix<float, float, 3>{fixed_zeros<float, 3, 3>(), p};}),
            std::move(camera),
            args.has_named("--rotate"),
            safe_stof(args.named_value("--scale", "1")));
    } catch (const CommandLineArgumentError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
