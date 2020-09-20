#include <Mlib/Arg_Parser.hpp>
#include <stb_image/stb_image_load.h>
#include <stb_image/stb_image_write.h>
#include <stb_image/stb_array.h>
#include <Mlib/Stats/Histogram_Matching.hpp>

using namespace Mlib;

int main(int argc, char **argv) {
    const ArgParser parser(
        "Usage: match_histograms --image image --ref ref --out <out>",
        {},
        {"--image", "--ref", "--out"});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unamed(0);
    StbInfo iimage = stb_load(args.named_value("--image"), false, false);
    StbInfo iref = stb_load(args.named_value("--ref"), false, false);
    Array<unsigned char> image = stb_image_2_array(iimage);
    Array<unsigned char> ref = stb_image_2_array(iref);
    Array<unsigned char> out = histogram_matching<unsigned char, unsigned char, float>(image.flattened(), ref.flattened(), 256);
    out.reshape(image.shape());
    std::unique_ptr<unsigned char> iout{new unsigned char[image.nelements()]};
    std::cerr << out[0][100].casted<float>() << std::endl;
    array_2_stb_image(out, iout.get());
    if (!stbi_write_png(
        args.named_value("--out").c_str(),
        iimage.width,
        iimage.height,
        iimage.nrChannels,
        iout.get(),
        0))
    {
        throw std::runtime_error("Could not write " + args.named_value("--out"));
    }
    return 0;
}
