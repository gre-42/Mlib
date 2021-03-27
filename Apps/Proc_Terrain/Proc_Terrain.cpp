#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/PgmImage.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Signal/Fft.hpp>
#include <Mlib/Stats/Arange.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <stb_image/stb_image_write.h>

using namespace Mlib;

/**
 * From: https://andrewwalker.github.io/statefultransitions/post/gaussian-fields/
 */
Array<float> fft_indgen(size_t n) {
    Array<float> a = arange<float>(n / 2 + 1);
    Array<float> b = -arange<float, size_t>(1, n / 2);
    return a.appended(b.reversed());
}

template <class TPk>
Array<float> gaussian_random_field(const TPk& Pk = [](float k){return std::pow(k, -3.f);}, size_t size = 100, unsigned int seed = 1) {
    auto Pk2 = [&Pk](float kx, float ky) -> float{
        if (kx == 0 && ky == 0) {
            return 0;
        }
        return std::sqrt(Pk(std::sqrt(squared(kx) + squared(ky))));
    };
    Array<std::complex<float>> noise = fft(normal_random_array<float>(ArrayShape{size, size}, seed).casted<std::complex<float>>());
    Array<float> amplitude{ArrayShape{size, size}};
    size_t i = 0;
    for (float kx : fft_indgen(size).flat_iterable()) {
        size_t j = 0;
        for (float ky : fft_indgen(size).flat_iterable()) {
            amplitude(i, j) = Pk2(kx, ky);
            ++j;
        }
        ++i;
    }
    return real(ifft(noise * amplitude.casted<std::complex<float>>()));
}

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: proc_terrain --heightmap <heightmap.pgm> --grf <grf.pgm> --binary_grf <binary_grf.array> --blended <blended.ppm> --size <size> --alpha <alpha> --min <min> --max <max> --seed <seed>",
        {},
        {"--heightmap", "--grf", "--binary_grf", "--blended", "--size", "--alpha", "--min", "--max", "--seed"});

    const auto args = parser.parsed(argc, argv);

    // Array<float> heightmap = PgmImage::load_from_file(args.named_value("--heightmap")).to_float();
    // Array<float> noise = normal_random_array<float>(heightmap.shape(), 1);
    // noise = lowpass_filter_NWE(noise, std::pow(3.f, 2.f) - Mlib::pow(linspace(-3.f, 3.f, 10), 2.f), NAN);
    // PgmImage res = PgmImage::from_float(maximum(0.f, gaussian_filter_NWE(100.f * laplace_filter(heightmap, NAN), 1.f, NAN)));
    // PgmImage res = PgmImage::from_float(heightmap);
    // PgmImage res = PgmImage::from_float(noise);
    // res.save_to_file(args.named_value("--result"));
    float alpha = safe_stof(args.named_value("--alpha"));
    Array<float> grf = gaussian_random_field(
        [alpha](float k){return std::pow(k, alpha);},
        safe_stoi(args.named_value("--size")),
        safe_stoi(args.named_value("--seed")));
    grf *= std::sqrt(grf.nelements());
    // std::cerr << min(grf) << " " << max(grf) << std::endl;
    grf = normalized_and_clipped(grf, safe_stof(args.named_value("--min")), safe_stof(args.named_value("--max")));
    // grf = normalized_and_clipped(grf);
    if (args.has_named_value("--grf")) {
        PgmImage::from_float(grf).save_to_file(args.named_value("--grf"));
    }
    if (args.has_named_value("--binary_grf")) {
        grf.save_binary(args.named_value("--binary_grf"));
    }
    if (args.has_named_value("--blended")) {
        Array<float> green = PpmImage{grf.shape(), Rgb24{100, 106, 32}}.to_float_rgb();
        Array<float> brown = PpmImage{grf.shape(), Rgb24{175, 146, 105}}.to_float_rgb();
        Array<float> res{ArrayShape{3}.concatenated(grf.shape())};
        for (size_t h = 0; h < 3; ++h) {
            res[h] = green[h] * grf + brown[h] * (1.f - grf);
        }
        PpmImage::from_float_rgb(res).save_to_file(args.named_value("--blended"));
    }
    // Array<std::complex<float>> d = normal_random_complex_array<float>(ArrayShape{10, 3}, 1);
    // std::cerr << d << std::endl;
    // std::cerr << ifft(fft(d)) << std::endl;
    // fft_inplace(d);
    // ifft_inplace(d);
    // std::cerr << d << std::endl;

    // auto dd = Array<std::complex<float>>{{1, 2, 3, 4}, {6, 2, 8, 9}}; //real(d).casted<std::complex<float>>();
    // std::cerr << real(dd) << std::endl;
    // std::cerr << fft(dd) << std::endl;
    return 0;
}
