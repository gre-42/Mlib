#include "Assemble_Tiles.hpp"
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Alpha_Channel_Mode.hpp>
#include <Mlib/Images/Compression/Add_Alpha_Channel.hpp>
#include <Mlib/Images/Compression/Multiply_Alpha_Channel.hpp>
#include <Mlib/Images/Compression/Tile_Image_Canvas.hpp>
#include <Mlib/Images/Compression/Tile_Image_File.hpp>
#include <Mlib/Images/Match_Rgba_Histograms.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/StbImage1.hpp>
#include <Mlib/Images/StbImage2.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Images/StbImage4.hpp>
#include <Mlib/Images/To_From_Multichannel.hpp>
#include <Mlib/Images/Transform/Resize.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Stats/Halton_Sequence.hpp>

using namespace Mlib;

Array<float> Mlib::assemble_tiles_compute_ols(FragmentAssembly& fa) {
    auto alpha_channel_mode = [&](){
        switch (fa.channels) {
        case 1:
        case 3:
            if (fa.add) {
                throw std::runtime_error("Add requires alpha channels");
            }
            return AlphaChannelMode::OFF;
        case 2:
        case 4:
            if (fa.add) {
                return AlphaChannelMode::ADD;
            } else {
                return AlphaChannelMode::BLEND;
            }
        }
        throw std::runtime_error("Unexpected number of channels");
    }();
    auto upsampled_width = fa.size(0) * fa.upsampling;
    auto upsampled_height = fa.size(1) * fa.upsampling;
    TileImageCanvas tile_image_canvas{
        upsampled_width,
        upsampled_height,
        fa.channels,
        alpha_channel_mode};
    Array<float> alpha;
    Array<float> alpha_fac;
    if (!fa.alpha.empty()) {
        alpha = StbImage1::load_from_file(fa.alpha.local_path()).to_float_grayscale();
    }
    if (!fa.alpha_fac.empty()) {
        alpha_fac = StbImage1::load_from_file(fa.alpha_fac.local_path()).to_float_grayscale();
    }
    Array<float> fragment;
    auto set_fragment = [&](const Array<float>& color){
        auto up = (fa.upsampling == 1)
            ? to_multichannel_2d(color)
            : enlarged_multichannel_2d(to_multichannel_2d(color), fa.upsampling);
        if (alpha.initialized()) {
            fragment = add_alpha_channel(up, alpha);
        } else {
            fragment = up;
        }
        if (alpha_fac.initialized()) {
            fragment = multiply_alpha_channel(fragment, alpha_fac);
        }
    };
    {
        switch (fa.channels - uint32_t(alpha.initialized())) {
        case 1:
            set_fragment(StbImage1::load_from_file(fa.color.local_path()).to_float_grayscale());
            break;
        case 2:
            set_fragment(StbImage2::load_from_file(fa.color.local_path()).to_float_ia());
            break;
        case 3:
            set_fragment(StbImage3::load_from_file(fa.color.local_path()).to_float_rgb());
            break;
        case 4:
            set_fragment(StbImage4::load_from_file(fa.color.local_path()).to_float_rgba());
            break;
        default:
            throw std::runtime_error("Unexpected number of channels");
        }
    }
    {
        float upsampled_stepsize = fa.stepsize * integral_to_float<float>(fa.upsampling);
        float upsampled_randsize = fa.randsize * integral_to_float<float>(fa.upsampling);
        HybridHaltonSequence<float> rng{42};
        for (float x = 0; x < integral_to_float<float>(upsampled_width); x += upsampled_stepsize) {
            for (float y = 0; y < integral_to_float<float>(upsampled_height); y += upsampled_stepsize) {
                tile_image_canvas.add(
                    fragment,
                    x + (rng() - 0.5f) * upsampled_randsize,
                    y + (rng() - 0.5f) * upsampled_randsize,
                    rng() * 2.f * (float)M_PI);
            }
        }
    }
    auto down = (fa.upsampling == 1)
        ? tile_image_canvas.canvas()
        : compressed_multichannel_2d(tile_image_canvas.canvas(), fa.upsampling, FilterExtension::PERIODIC);
    if (fa.ols.has_value()) {
        if (fa.ols->empty()) {
            fa.ols->reserve(down.shape(0));
            auto hpms = match_rgba_histograms(down, fragment);
            for (const auto& hm : hpms.hms) {
                auto ols = hm.ols();
                fa.ols->emplace_back(ols.offset(), ols.slope());
                // linfo() << "OLS: offset = " << ols.offset() << ", slope = " << ols.slope();
            }
        }
        if (down.shape(0) == 0) {
            throw std::runtime_error("Number of channels is zero");
        }
        if (fa.ols->size() != down.shape(0) - (1 - down.shape(0) % 2)) {
            throw std::runtime_error("Incorrect number of OLS coefficients");
        }
        for (const auto& [h, c] : enumerate(*fa.ols)) {
            down[h] = clipped(c.offset + c.slope * down[h], 0.f, 1.f);
        }
        // down = hpms.matched;
    }
    return to_singlechannel_2d(down);
}

Array<float> Mlib::assemble_tiles(const FragmentAssembly& fa) {
    if (fa.ols.has_value() && fa.ols->empty()) {
        throw std::runtime_error("OLS coefficents missing");
    }
    return assemble_tiles_compute_ols(const_cast<FragmentAssembly&>(fa));
}