#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Render/Render.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <vector>

using namespace Mlib;

static const std::vector<ColoredVertex> vertices{
    { FixedArray<float, 3>{-0.6f, -0.4f, 0.f}, FixedArray<float, 3>{1.f, 0.f, 0.f}},
    { FixedArray<float, 3>{0.6f, -0.4f, 0.f}, FixedArray<float, 3>{0.f, 1.f, 0.f}},
    { FixedArray<float, 3>{0.f,  0.6f, 0.f}, FixedArray<float, 3>{0.f, 0.f, 1.f}}
};

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: render_depth_map --rgb <filename> --depth <filename> --ki <intrinsic_matrix> [--z_offset <z_offset>] [--rotate]",
        {"--rotate"},
        {"--rgb", "--depth", "--ki", "--z_offset"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unamed(0);

        // render(vertices);

        PpmImage img = PpmImage::load_from_file(args.named_value("--rgb"));
        Array<float> depth = Array<float>::load_binary(args.named_value("--depth"));
        Array<float> intrinsic_matrix = Array<float>::load_txt_2d(args.named_value("--ki"));
        if (!all(intrinsic_matrix.shape() == ArrayShape{3, 3})) {
            throw std::runtime_error("Intrinsic matrix has incorrect shape");
        }
        if (!all(depth.shape() == img.shape())) {
            throw std::runtime_error("Depth and image shape differ");
        }
        ::Mlib::render_depth_map(
            img.to_float_rgb(),
            depth,
            FixedArray<float, 3, 3>{intrinsic_matrix},
            safe_stof(args.named_value("--z_offset", "1")),
            args.has_named("--rotate"));
    } catch (const CommandLineArgumentError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
