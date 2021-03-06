#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geometry/Coordinates/Normalized_Points_Fixed.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/PgmImage.hpp>
#include <Mlib/Images/Resample/Pyramid.hpp>
#include <Mlib/Images/StbImage.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Render/Normal_Type.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Render_Height_Map.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <stb_image/stb_array.h>
#include <stb_image/stb_image_load.hpp>
#include <vector>

using namespace Mlib;

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: render_height_map [--rgb <filename.png>] --height <filename.{pgm,jpg,png}> [--xy_scale <scale>] [--z_scale <scale>] [--rotate] [--down_sample <n>] [--normal_type {face,vertex}]",
        {"--rotate"},
        {"--rgb", "--height", "--xy_scale", "--z_scale", "--down_sample", "--normal_type"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unamed(0);

        Array<float> height;
        if (args.named_value("--height").ends_with(".pgm")) {
            height = PgmImage::load_from_file(args.named_value("--height")).to_float() / 64.f * float(UINT16_MAX);
        } else {
            Array<float> im = stb_image_2_array(stb_load(args.named_value("--height"), false, false)).casted<float>();
            if (im.shape(0) == 3) {
                // https://www.mapzen.com/blog/elevation/
                height = im[0] * 256.f + im[1] + im[2] / 256.f - 32768.f;
            } else if (im.shape(0) == 1) {
                height = im[0] / 255.f;
            } else {
                throw std::runtime_error("Height map is no PGM image and does not have 1 or 3 channels");
            }
        }
        Array<float> color = args.has_named_value("--rgb")
            ? StbImage::load_from_file(args.named_value("--rgb")).to_float_rgb()
            : StbImage{ height.shape(), Rgb24::white() }.to_float_rgb();
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
        SceneNodeResources scene_node_resources;
        RenderingContextGuard rrg{scene_node_resources, "primary_rendering_resources", 16, 0};
        size_t num_renderings = SIZE_MAX;
        RenderConfig render_config;
        Render2 render{ render_config, num_renderings };
        render_height_map(
            render,
            color,
            height * safe_stof(args.named_value("--z_scale", "0.001")),
            np.normalization_matrix().pre_scaled(safe_stof(args.named_value("--xy_scale", "1"))),
            normal_type_from_string(args.named_value("--normal_type", "face")),
            args.has_named("--rotate"));
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
