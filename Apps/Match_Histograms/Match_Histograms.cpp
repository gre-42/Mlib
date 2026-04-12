#include <Mlib/Images/Match_Rgba_Histograms.hpp>
#include <Mlib/Io/Arg_Parser.hpp>
#include <Mlib/Strings/Utf8_Path.hpp>
#include <stb/stb_image_write.h>
#include <stb_cpp/stb_array.hpp>
#include <stb_cpp/stb_image_load.hpp>
#include <stdexcept>

using namespace Mlib;

Array<unsigned char> safe_load_rgb(const Utf8Path& filename) {
    StbInfo iimage = stb_load8(filename, FlipMode::NONE);
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
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed(0);
        Array<unsigned char> image = safe_load_rgb(args.named_value("--image"));
        Array<unsigned char> ref = safe_load_rgb(args.named_value("--ref"));
        auto hpms = match_rgba_histograms(image, ref);
        std::unique_ptr<unsigned char[]> iout{new unsigned char[image.nelements()]};
        array_2_stb_image(hpms.matched, iout.get());
        if (any(image.shape() > INT_MAX)) {
            throw std::runtime_error("Image size too large");
        }
        auto out_path = Utf8Path(args.named_value("--out"));
        if (!stbi_write_png(
            out_path.c_str(),
            (int)image.shape(2),
            (int)image.shape(1),
            (int)image.shape(0),
            iout.get(),
            0))
        {
            throw std::runtime_error("Could not write \"" + out_path.string() + '"');
        }
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
