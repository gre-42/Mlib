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

using namespace Mlib;

/**
 * From: https://andrewwalker.github.io/statefultransitions/post/gaussian-fields/
 */
Array<size_t> fft_indgen(size_t n) {
    Array<size_t> a = arange<size_t>(n / 2 + 1);
    Array<size_t> b = n - arange<size_t, size_t>(1, n / 2);
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
    for (size_t kx : fft_indgen(size).flat_iterable()) {
        size_t j = 0;
        for (size_t ky : fft_indgen(size).flat_iterable()) {
            amplitude(i, j) = Pk2(kx, ky);
            ++j;
        }
        ++i;
    }
    return real(ifft(noise * amplitude.casted<std::complex<float>>()));
}

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: proc_terrain --heightmap <heightmap.pgm> --result <result.ppm> --seed <seed>",
        {},
        {"--heightmap", "--result", "--seed"});

    const auto args = parser.parsed(argc, argv);

    Array<float> heightmap = PgmImage::load_from_file(args.named_value("--heightmap")).to_float();
    Array<float> noise = normal_random_array<float>(heightmap.shape(), 1);
    noise = lowpass_filter_NWE(noise, std::pow(3.f, 2.f) - Mlib::pow(linspace(-3.f, 3.f, 10), 2.f), NAN);
    // PgmImage res = PgmImage::from_float(maximum(0.f, gaussian_filter_NWE(100.f * laplace_filter(heightmap, NAN), 1.f, NAN)));
    // PgmImage res = PgmImage::from_float(heightmap);
    PgmImage res = PgmImage::from_float(noise);
    res.save_to_file(args.named_value("--result"));
    for (float alpha : Array<float>{-4.0, -3.0, -2.0}.flat_iterable()) {
        Array<float> out = gaussian_random_field([alpha](float k){return std::pow(k, alpha);}, 256);
        out = normalized_and_clipped(out);
        PgmImage::from_float(out).save_to_file("/tmp/grf-" + std::to_string(alpha) + ".pgm");
    }
    return 0;
}
