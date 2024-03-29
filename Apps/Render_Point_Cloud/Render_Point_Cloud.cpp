#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Cv/Render/Render_Data.hpp>
#include <Mlib/Geometry/Cameras/Perspective_Camera.hpp>
#include <Mlib/Geometry/Coordinates/Normalized_Points_Fixed.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Pgm_Image.hpp>
#include <Mlib/Images/Ppm_Image.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Render/Particle_Resources.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/To_Number.hpp>
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

        args.assert_num_unnamed(0);

        Array<FixedArray<float, 3>> points = Array<float>::from_dynamic<3>(Array<float>::load_txt_2d(args.named_value("--points")));
        SceneNodeResources scene_node_resources;
        ParticleResources particle_resources;
        auto rrg = RenderingContextGuard::root(
            scene_node_resources,
            particle_resources,
            "primary_rendering_resources",
            16,
            0);
        std::atomic_size_t num_renderings = SIZE_MAX;
        RenderConfig render_config;
        std::unique_ptr<Camera> camera(new PerspectiveCamera(
            PerspectiveCameraConfig(),
            PerspectiveCamera::Postprocessing::ENABLED));
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
