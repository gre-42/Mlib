#include <Mlib/Images/Svg.hpp>
#include <string>
#include <iosfwd>
#include <fstream>
#include <random>

using namespace Mlib;

void generate_bush(
    std::ostream& ostr,
    size_t width,
    size_t height,
    const std::string& twig_filename,
    size_t twig_width,
    size_t twig_height)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<float> dist(0, 1);

    Svg svg{ostr, width, height};
    float border_x = 100 + twig_width / 2;
    float border_y = 100 + twig_height / 2;
    for(size_t i = 0; i < 10; ++i) {
        SvgTransform t{ostr, SvgTransformationParameters<float>{
            angle: float((dist(rng) - 0.5) * 45),
            rotation_x: float(twig_width / 2),
            rotation_y: float(twig_height / 2),
            translation_x: border_x + float(dist(rng) * (width - 2 * border_x) - twig_width / 2),
            translation_y: border_y + float(dist(rng) * (height- 2 * border_y) - twig_height / 2)}};
        svg.draw_image(twig_filename, twig_width, twig_height);
        t.finish();
    }
    svg.finish();
}

// int main() {
//     std::ofstream f{"bush.svg"};
//     generate_bush(f, 800, 600, "twig.png", 50 * 8, 50 * 6);
//     return 0;
// }
