#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/StbImage.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <vector>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Render/Rendering_Context.hpp>

using namespace Mlib;

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: render_depth_map --rgb <filename> --depth <filename> --ki <intrinsic_matrix> [--z_offset <z_offset>] [--rotate]",
        {"--rotate"},
        {"--rgb", "--depth", "--ki", "--z_offset"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unamed(0);

        StbImage img = StbImage::load_from_file(args.named_value("--rgb"));
        Array<float> depth = Array<float>::load_binary(args.named_value("--depth"));
        Array<float> intrinsic_matrix = Array<float>::load_txt_2d(args.named_value("--ki"));
        if (!all(intrinsic_matrix.shape() == ArrayShape{3, 3})) {
            throw std::runtime_error("Intrinsic matrix has incorrect shape");
        }
        if (!all(depth.shape() == img.shape())) {
            throw std::runtime_error("Depth and image shape differ");
        }
        size_t num_renderings = SIZE_MAX;
        RenderConfig render_config;
        SceneNodeResources scene_node_resources;
        RenderingContextGuard rrg{scene_node_resources, "primary_rendering_resources", render_config.anisotropic_filtering_level, 0};
        Render2{ render_config, num_renderings }.render_depth_map(
            img.to_float_rgb(),
            depth,
            TransformationMatrix<float, 2>{ FixedArray<float, 3, 3>{ intrinsic_matrix } },
            safe_stof(args.named_value("--z_offset", "1")),
            args.has_named("--rotate"));
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
