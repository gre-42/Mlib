#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Filters/Guided_Filter.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/Ppm_Image.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <stb/stb_image_resize2.h>
#include <stb_cpp/stb_array.hpp>
#include <stb_cpp/stb_image_load.hpp>
#include <vector>

using namespace Mlib;

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: generate_tree --mask <mask> --guidance <guidance> --output <output.ppm> --box-size <box-size> --eps <eps> --niter <niter>",
        {},
        {"--mask", "--guidance", "--output", "--box-size", "--eps", "--niter"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unnamed(0);

        auto mask = stb_image_2_array(stb_load8(args.named_value("--mask"), FlipMode::VERTICAL)).casted<float>() / 255.f;
        auto mask_gray = sum(mask, 0) / float(mask.shape(0));
        auto guidance = stb_image_2_array(stb_load8(args.named_value("--guidance"), FlipMode::VERTICAL)).casted<float>() / 255.f;
        auto guidance_gray = sum(guidance, 0) / float(guidance.shape(0));
        size_t b = safe_stoz(args.named_value("--box-size"));
        if (any(mask_gray.shape() != guidance_gray.shape())) {
            throw std::runtime_error("Images do not have identical sizes");
        }
        auto filt = mask_gray;
        for (size_t i = 0; i < safe_stoz(args.named_value("--niter")); ++i) {
            filt = guided_filter(guidance_gray, filt, ArrayShape{b, b}, float(safe_stof(args.named_value("--eps"))));
        }
        PpmImage::from_float_rgb(clipped(Array<float>({filt, filt, filt}), 0.f, 1.f)).save_to_file(args.named_value("--output"));
    } catch (const CommandLineArgumentError& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
