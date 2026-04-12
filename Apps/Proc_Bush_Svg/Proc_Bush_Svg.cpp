#include <Mlib/Images/Svg.hpp>
#include <Mlib/Io/Arg_Parser.hpp>
#include <Mlib/Os/Utf8_Path.hpp>
#include <Mlib/Strings/String_View_To_Number.hpp>
#include <fstream>
#include <iosfwd>
#include <random>
#include <stdexcept>
#include <string>

using namespace Mlib;

void generate_bush(
    std::ostream& ostr,
    float width,
    float height,
    const Utf8Path& twig_filename,
    float twig_width,
    float twig_height,
    size_t ntwigs,
    unsigned int seed)
{
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(0, 1);

    Svg svg{ostr, width, height};
    float border_x = 100.f + twig_width / 2.f;
    float border_y = 100.f + twig_height / 2.f;
    for (size_t i = 0; i < ntwigs; ++i) {
        SvgTransform t{ostr, SvgTransformationParameters<float>{
            .angle = (dist(rng) - 0.5f) * 45.f,
            .rotation_x = twig_width / 2.f,
            .rotation_y = twig_height / 2.f,
            .translation_x = border_x + (dist(rng) * (width - 2.f * border_x) - twig_width / 2.f),
            .translation_y = border_y + (dist(rng) * (height- 2.f * border_y) - twig_height / 2.f)}};
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

    std::string svg_filename = Utf8Path{args.named_value("--result")};
    std::ofstream f{svg_filename};
    generate_bush(
        f,
        800.f,
        600.f,
        args.named_value("--twig"),
        safe_stof(args.named_svalue("--twig_width")),
        safe_stof(args.named_svalue("--twig_height")),
        safe_stoz(args.named_svalue("--ntwigs")),
        safe_stou(args.named_svalue("--seed", "0")));
    f.flush();
    if (f.fail()) {
        throw std::runtime_error("Could not write " + svg_filename);
    }
    return 0;
}
