#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geometry/Normalized_Points_Fixed.cpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/PgmImage.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Render/Render.hpp>
#include <Mlib/String.hpp>
#include <vector>

using namespace Mlib;

static const std::vector<ColoredVertex> vertices{
    { FixedArray<float, 3>{-0.6f, -0.4f, 0.f}, FixedArray<float, 3>{1.f, 0.f, 0.f}},
    { FixedArray<float, 3>{0.6f, -0.4f, 0.f}, FixedArray<float, 3>{0.f, 1.f, 0.f}},
    { FixedArray<float, 3>{0.f,  0.6f, 0.f}, FixedArray<float, 3>{0.f, 0.f, 1.f}}
};

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: render_depth_map --rgb <filename.ppm> --height <filename.pgm> [--xy_scale <scale>] [--z_scale <scale>] [--rotate]",
        {"--rotate"},
        {"--rgb", "--height", "--xy_scale", "--z_scale"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unamed(0);

        // render(vertices);

        PpmImage img = PpmImage::load_from_file(args.named_value("--rgb"));
        PgmImage height = PgmImage::load_from_file(args.named_value("--height"));
        if (!all(height.shape() == img.shape())) {
            throw std::runtime_error("Depth and image shape differ");
        }
        NormalizedPointsFixed np{ScaleMode::PRESERVE_ASPECT_RATIO, OffsetMode::CENTERED};
        np.add_point({0.f, 0.f});
        np.add_point({float(img.shape(id1)) - 1, float(img.shape(id0)) - 1});
        ::Mlib::render_height_map(
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
