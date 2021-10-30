#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Cv/Render/Render_Data.hpp>
#include <Mlib/Geometry/Coordinates/Normalized_Points_Fixed.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/PgmImage.hpp>
#include <Mlib/Images/StbImage.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <vector>

using namespace Mlib;
using namespace Mlib::Cv;

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: render_height_map --rgb <filename.png> --height <filename.pgm> [--xy_scale <scale>] [--z_scale <scale>] [--rotate]",
        {"--rotate"},
        {"--rgb", "--height", "--xy_scale", "--z_scale"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unamed(0);

        Array<float> height;
        if (args.named_value("--height").ends_with(".pgm")) {
            height = PgmImage::load_from_file(args.named_value("--height")).to_float();
        } else {
            auto im_rgb = StbImage::load_from_file(args.named_value("--height")).to_float_rgb() * 255.f;
            if (im_rgb.shape(0) != 3) {
                throw std::runtime_error("Height map is no PGM image and does not have 3 channels");
            }
            // https://www.mapzen.com/blog/elevation/
            height = im_rgb[0] * 256.f + im_rgb[1] + im_rgb[2] / 256.f - 32768.f;
        }
        StbImage img = args.has_named_value("--rgb")
            ? StbImage::load_from_file(args.named_value("--rgb"))
            : StbImage{ height.shape(), Rgb24::white() };
        if (!all(height.shape() == img.shape())) {
            throw std::runtime_error("Depth and image shape differ");
        }
        NormalizedPointsFixed np{ScaleMode::PRESERVE_ASPECT_RATIO, OffsetMode::CENTERED};
        np.add_point({0.f, 0.f});
        np.add_point({float(img.shape(id1)) - 1, float(img.shape(id0)) - 1});
        SceneNodeResources scene_node_resources;
        RenderingContextGuard rrg{scene_node_resources, "primary_rendering_resources", 16, 0};
        size_t num_renderings = SIZE_MAX;
        RenderConfig render_config;
        Render2 render{ render_config, num_renderings };
        render_height_map(
            render,
            img.to_float_rgb(),
            height * safe_stof(args.named_value("--z_scale", "1")),
            np.normalization_matrix().pre_scaled(safe_stof(args.named_value("--xy_scale", "1"))),
            args.has_named("--rotate"));
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
