#include <Mlib/Arg_Parser.hpp>
#include <stb_image/stb_image_load.h>
#include <stb_image/stb_image_write.h>
#include <stb_image/stb_array.h>
#include <Mlib/Stats/Histogram_Matching.hpp>

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
    Array<unsigned char> out{image.shape()};
    if (image.shape(0) != ref.shape(0)) {
        throw std::runtime_error("Images have different number of channels");
    }
    if (image.shape(0) == 3) {
        for(size_t d = 0; d < 3; ++d) {
            out[d] = histogram_matching<unsigned char, unsigned char, float>(image[d].flattened(), ref[d].flattened(), 256);
        }
    } else if (image.shape(0) == 4) {
        Array<bool> mask = (image[3] > (unsigned char)50);
        Array<bool> mask_ref = (ref[3] > (unsigned char)50);
        for(size_t d = 0; d < 3; ++d) {
            HistogramMatching<unsigned char, unsigned char, float> hm{image[d][mask], ref[d][mask_ref], 256};
            for(size_t r = 0; r < image.shape(1); ++r) {
                for(size_t c = 0; c < image.shape(2); ++c) {
                    out(d, r, c) = hm(image(d, r, c), true);
                }
            }
        }
        if (image.shape(0) == 4) {
            out[3] = image[3];
        }
    } else {
        assert_true(false);
    }
    out.reshape(image.shape());
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
