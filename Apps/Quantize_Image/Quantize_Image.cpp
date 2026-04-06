#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/StbImage1.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Images/StbImage4.hpp>
#include <Mlib/Io/Arg_Parser.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <Mlib/Misc/Floating_Point_Exceptions.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

struct KMeansImages {
    KMeansImages(
        size_t k,
        float std,
        const Array<float>& colors,
        const Array<float>& alpha,
        size_t niterations)
    {
        if (colors.ndim() != 3) {
            throw std::runtime_error("Color image does not have 3 dimensions");
        }
        if (alpha.ndim() != 2) {
            throw std::runtime_error("Alpha image does not have 2 dimensions");
        }
        if (k > 500) {
            throw std::runtime_error("k is too large");
        }
        if (niterations == 0) {
            throw std::runtime_error("niterations must be >= 1");
        }
        auto nchannels = colors.shape(0);
        auto spatial_dimensions = colors.shape().erased_first();
        if (any(spatial_dimensions != alpha.shape())) {
            throw std::runtime_error("Alpha shape differs from color shape");
        }
        probabilities = Array<float>{ArrayShape{k}.concatenated(spatial_dimensions)};
        means = Array<float>{ArrayShape{k, nchannels}};
        {
            auto L = Linspace<float>{0.f, 1.f, k};
            for (size_t channel = 0; channel < nchannels; ++channel) {
                for (size_t cluster = 0; cluster < k; ++cluster) {
                    means(cluster, channel) = L[cluster];
                }
            }
        }
        auto distances = Array<float>{probabilities.shape()};
        for (size_t i = 0; i < niterations; ++i) {
            for (size_t cluster = 0; cluster < k; ++cluster) {
                distances[cluster] = 0;
                for (size_t channel = 0; channel < nchannels; ++channel) {
                    distances[cluster] += abs(colors[channel] - means(cluster, channel));
                }
            }
            {
                auto p = exp(-distances / std / integral_to_float<float>(nchannels));
                auto psum = sum(p, 0);
                for (size_t cluster = 0; cluster < k; ++cluster) {
                    probabilities[cluster] = p[cluster] / psum;
                }
            }
            {
                for (size_t cluster = 0; cluster < k; ++cluster) {
                    auto weight = alpha * probabilities[cluster];
                    auto wsum = sum(weight);
                    for (size_t channel = 0; channel < nchannels; ++channel) {
                        means(cluster, channel) = sum(colors[channel] * weight) / wsum;
                    }
                }
            }
        }
    }
    Array<float> reconstructed(float gamma) {
        if (probabilities.ndim() != 3) {
            throw std::runtime_error("Unexpected shape of the probability array");
        }
        if (means.ndim() != 2) {
            throw std::runtime_error("Unexpected shape of the means array");
        }
        if (probabilities.shape(0) != means.shape(0)) {
            throw std::runtime_error("Inconsistent number of clusters");
        }
        auto k = probabilities.shape(0);
        auto nchannels = means.shape(1);
        auto spatial_dimensions = probabilities.shape().erased_first();
        auto pg = pow(probabilities, gamma);
        auto pgsum = sum(pg, 0);
        auto reconstructed = zeros<float>(ArrayShape{nchannels}.concatenated(spatial_dimensions));
        for (size_t channel = 0; channel < nchannels; ++channel) {
            for (size_t cluster = 0; cluster < k; ++cluster) {
                reconstructed[channel] += pg[cluster] * means(cluster, channel);
            }
            reconstructed[channel] /= pgsum;
        }
        return reconstructed;
    }
    Array<float> probabilities;
    Array<float> means;
};

int main(int argc, char** argv) {
    enable_floating_point_exceptions();

    const char* help =
        "Usage: quantize_image "
        "[--help] "
        "--k <k> "
        "--std <std> "
        "--iterations <n> "
        "--source <file> "
        "--dest_p_prefix <prefix> "
        "[--dest_reconstructed <filename> --gamma <gamma>]";
    const ArgParser parser(
        help,
        {"--help"},
        {"--k",
         "--std",
         "--iterations",
         "--source",
         "--dest_p_prefix",
         "--dest_reconstructed",
         "--gamma"});
    try {
        const auto args = parser.parsed(argc, argv);
        if (args.has_named("--help")) {
            lout() << help;
            return 0;
        }
        args.assert_num_unnamed(0);
        auto src = args.named_value("--source");
        auto image = stb_load8(src, FlipMode::NONE);
        Array<float> colors;
        Array<float> alpha;
        if (image.nrChannels == 3) {
            auto colors3 = StbImage3{image};
            colors = colors3.to_float_rgb();
            alpha = ones<float>(colors3.shape());
        } else if (image.nrChannels == 4) {
            auto colors4 = StbImage4{image};
            auto colors4f = colors4.to_float_rgba();
            colors = colors4f.row_range(0, 3);
            alpha = colors4f[3];
        } else {
            throw std::runtime_error("Unsupported number of channels");
        }
        auto k_means_images = KMeansImages{
            safe_stoz(args.named_value("--k")),
            safe_stof(args.named_value("--std")),
            colors,
            alpha,
            safe_stoz(args.named_value("--iterations"))};
        if (auto dest_p_prefix = args.try_named_value("--dest_p_prefix"); dest_p_prefix != nullptr) {
            for (const auto& [i, p] : enumerate(k_means_images.probabilities)) {
                StbImage1::from_float_grayscale(p).save_to_file(*dest_p_prefix + std::to_string(i) + ".png");
            }
        }
        if (auto dest_reconstructed = args.try_named_value("--dest_reconstructed"); dest_reconstructed != nullptr) {
            auto gamma = safe_stof(args.named_value("--gamma"));
            auto recon = k_means_images.reconstructed(gamma);
            if (image.nrChannels == 3) {
                StbImage3::from_float_rgb(recon).save_to_file(*dest_reconstructed);
            } else if (image.nrChannels == 4) {
                StbImage4::from_float_rgba(Array<float>{recon[0], recon[1], recon[2], alpha}).save_to_file(*dest_reconstructed);
            } else {
                throw std::runtime_error("Unsupported number of channels");
            }
        }
    } catch (const std::exception& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
