#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/String.cpp>
#include <fstream>
#include <iosfwd>
#include <random>
#include <stdexcept>
#include <string>

using namespace Mlib;

void generate_bush(
    std::ostream& ostr,
    size_t width,
    size_t height,
    const std::string& twig_filename,
    size_t twig_width,
    size_t twig_height,
    size_t ntwigs,
    unsigned int seed)
{
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(0, 1);

    Svg svg{ostr, width, height};
    float border_x = 100 + twig_width / 2;
    float border_y = 100 + twig_height / 2;
    for (size_t i = 0; i < ntwigs; ++i) {
        SvgTransform t{ostr, SvgTransformationParameters<float>{
            .angle = float((dist(rng) - 0.5) * 45),
            .rotation_x = float(twig_width / 2),
            .rotation_y = float(twig_height / 2),
            .translation_x = border_x + float(dist(rng) * (width - 2 * border_x) - twig_width / 2),
            .translation_y = border_y + float(dist(rng) * (height- 2 * border_y) - twig_height / 2)}};
        svg.draw_image(twig_filename, twig_width, twig_height);
        t.finish();
    }
    svg.finish();
}

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: proc_bush_svg --twig <twig.png> --twig_width <twig_width> --twig_height <twig_height> --ntwigs <ntwigs> --result <result.svg> --seed <seed>",
        {},
        {"--twig", "--twig_width", "--twig_height", "--ntwigs", "--result", "--seed"});

    const auto args = parser.parsed(argc, argv);

    std::string svg_filename = args.named_value("--result");
    std::ofstream f{svg_filename};
    generate_bush(
        f,
        800,
        600,
        args.named_value("--twig"),
        safe_stoi(args.named_value("--twig_width")),
        safe_stoi(args.named_value("--twig_height")),
        safe_stoi(args.named_value("--ntwigs")),
        safe_stoi(args.named_value("--seed", "0")));
    f.flush();
    if (f.fail()) {
        throw std::runtime_error("Could not write " + svg_filename);
    }
    return 0;
}
