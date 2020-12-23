#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geometry/Normalized_Points_Fixed.cpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/PgmImage.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/String.hpp>
#include <vector>

using namespace Mlib;

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: render_depth_map --rgb <filename.ppm> --height <filename.pgm> [--xy_scale <scale>] [--z_scale <scale>] [--rotate]",
        {"--rotate"},
        {"--rgb", "--height", "--xy_scale", "--z_scale"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unamed(0);

        PpmImage img = PpmImage::load_from_file(args.named_value("--rgb"));
        PgmImage height = PgmImage::load_from_file(args.named_value("--height"));
        if (!all(height.shape() == img.shape())) {
            throw std::runtime_error("Depth and image shape differ");
        }
        NormalizedPointsFixed np{ScaleMode::PRESERVE_ASPECT_RATIO, OffsetMode::CENTERED};
        np.add_point({0.f, 0.f});
        np.add_point({float(img.shape(id1)) - 1, float(img.shape(id0)) - 1});
        RenderingResources rendering_resources;
        size_t num_renderings = SIZE_MAX;
        Render2{num_renderings}.render_height_map(
            rendering_resources,
            img.to_float_rgb(),
            height.to_float() * safe_stof(args.named_value("--z_scale", "1")),
            np.normalization_matrix() * safe_stof(args.named_value("--xy_scale", "1")),
            args.has_named("--rotate"));
    } catch (const CommandLineArgumentError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
