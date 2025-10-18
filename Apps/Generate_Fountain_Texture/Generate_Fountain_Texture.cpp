#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Images/Filters/Gaussian_Kernel.hpp>
#include <Mlib/Images/StbImage4.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Stats/Random_Unit_Vector_Generator.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <stb_cpp/stb_generate_color_mask.hpp>
#include <stb_cpp/stb_transform.hpp>

using namespace Mlib;

int main(int argc, char** argv) {
    enable_floating_point_exceptions();
    const ArgParser parser(
        "Usage: generate_fountain_texture destination_prefix -r <r> -g <g> -b <b> --width <width> --height <height> --nimages <n>",
        {},
        {"-r",
         "-g",
         "-b",
         "--width",
         "--height",
         "--nimages"});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed(1);
        auto destination_prefix = args.unnamed_value(0);
        auto out_alpha = Array<float>(ArrayShape{
            safe_stoz(args.named_value("--height", "256")),
            safe_stoz(args.named_value("--width", "256"))});
        auto nimages = safe_stoz(args.named_value("--nimages", "5"));
        auto color = FixedArray<float, 3>{
            safe_stof(args.named_value("-r", "1")),
            safe_stof(args.named_value("-g", "1")),
            safe_stof(args.named_value("-b", "1"))};
        size_t nparticles = 50;
        float initial_velocity_mean = 400 * cm / seconds;
        float initial_velocity_std = 300 * cm / seconds;
        float gravity = 9.8f * meters / squared(seconds);
        float dpi = 96.f / meters;
        float particle_sigma = 9 * cm;
        float intensity_scaling = 1.f;
        float fadeout_sigma = 60 * cm;
        float dt = 4.f / 60.f * seconds;
        std::vector<VectorAtPosition<float, float, 2>> particles(
            nparticles,
            VectorAtPosition<float, float, 2>{fixed_nans<float, 2>(), fixed_zeros<float, 2>() * meters});
        {
            RandomUnitVectorGenerator<float, 2> velocity_direction_rng{0};
            FastNormalRandomNumberGenerator<float> velocity_magnitued_rng{0, initial_velocity_mean, initial_velocity_std};
            for (auto& p : particles) {
                p.vector = velocity_direction_rng() * velocity_magnitued_rng();
            }
        }
        auto kernel1d = gaussian_kernel(particle_sigma * dpi, 4.f, ShapeType::FULL, NormalizationType::MAX);
        auto kernel = Array<float>{ArrayShape{ kernel1d.length(), kernel1d.length() }};
        for (size_t r = 0; r < kernel1d.length(); ++r) {
            for (size_t c = 0; c < kernel1d.length(); ++c) {
                kernel(r, c) = kernel1d(r) * kernel1d(c);
            }
        }
        auto fadeout_kernel1d = gaussian_kernel(fadeout_sigma * dpi, 4.f, ShapeType::HALF, NormalizationType::MAX);
        linfo() << "Kernel size: " << kernel.shape();
        for (size_t i = 0; i < nimages; ++i) {
            out_alpha = 0.f;
            for (auto& p : particles) {
                for (size_t rs = 0; rs < kernel.shape(0); ++rs) {
                    for (size_t cs = 0; cs < kernel.shape(1); ++cs) {
                        float rt = std::round((float)out_alpha.shape(0) / 2 + (float)rs - (float)kernel.shape(0) / 2 + p.position(1) * dpi);
                        float ct = std::round((float)out_alpha.shape(1) / 2 + (float)cs - (float)kernel.shape(1) / 2 + p.position(0) * dpi);
                        if ((rt < 0) || (ct < 0)) {
                            continue;
                        }
                        auto rti = float_to_integral<size_t>(rt);
                        auto cti = float_to_integral<size_t>(ct);
                        if ((rti >= out_alpha.shape(0)) || (cti >= out_alpha.shape(1))) {
                            continue;
                        }
                        out_alpha(rti, cti) += kernel(rs, cs);
                    }
                }
            }
            for (size_t r = 0; r < out_alpha.shape(0); ++r) {
                for (size_t c = 0; c < out_alpha.shape(1); ++c) {
                    auto i = float_to_integral<size_t>(std::round(std::sqrt(
                        squared((double)r - (double)out_alpha.shape(0) / 2) +
                        squared((double)c - (double)out_alpha.shape(1) / 2))));
                    if (i >= fadeout_kernel1d.length()) {
                        out_alpha(r, c) = 0.f;
                    } else {
                        out_alpha(r, c) *= fadeout_kernel1d(i);
                    }
                }
            }
            auto out_image = StbImage4{out_alpha.applied<Rgba32>([&](auto& v){
                return Rgba32::from_float_rgba(color(0), color(1), color(2), std::min(v * intensity_scaling, 1.f));
            })};
            out_image.save_to_file(destination_prefix + std::to_string(i) + ".png");
            for (auto& p : particles) {
                p.position += p.vector * dt;
                p.vector(1) += gravity * dt;
            }
        }
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
