#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Match_Rgba_Histograms.hpp>
#include <stb_image/stb_array.h>
#include <stb_image/stb_image_load.h>
#include <stb_image/stb_image_write.h>

using namespace Mlib;

Array<unsigned char> safe_load_rgb(const std::string& filename) {
    StbInfo iimage = stb_load(filename, false, false);
    Array<unsigned char> image = stb_image_2_array(iimage);
    if (image.shape(0) != 3 && image.shape(0) != 4) {
        throw std::runtime_error("Dimension not 3 or 4");
    }
    return image;
}

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: match_histograms --image image --ref ref --out <out>",
        {},
        {"--image", "--ref", "--out"});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unamed(0);
    Array<unsigned char> image = safe_load_rgb(args.named_value("--image"));
    Array<unsigned char> ref = safe_load_rgb(args.named_value("--ref"));
    Array<unsigned char> out = match_rgba_histograms(image, ref);
    std::unique_ptr<unsigned char> iout{new unsigned char[image.nelements()]};
    array_2_stb_image(out, iout.get());
    if (!stbi_write_png(
        args.named_value("--out").c_str(),
        image.shape(2),
        image.shape(1),
        image.shape(0),
        iout.get(),
        0))
    {
        throw std::runtime_error("Could not write " + args.named_value("--out"));
    }
    return 0;
}
