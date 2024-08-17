#include "Raster_Factory.hpp"
#include <Mlib/Geometry/Mesh/Load/Load_Dff.hpp>
#include <Mlib/Geometry/Mesh/Load/Raster_Config.hpp>
#include <Mlib/Render/Raster/D3d8_Raster.hpp>
#include <Mlib/Render/Raster/Gl3_Raster.hpp>
#include <Mlib/Render/Raster/Ps2_Raster.hpp>

using namespace Mlib::Dff;

RasterFactory::RasterFactory()
{}

bool RasterFactory::is_p8_supported() const {
    return false;
}

std::unique_ptr<IRaster> RasterFactory::create_raster(
    const Image& img,
    uint32_t type,
    const RasterConfig& raster_config) const
{
    if ((type & 0xF) != Raster::TEXTURE) {
        THROW_OR_ABORT("Invalid raster type");
    }

    //    for(width = 1; width < img->width; width <<= 1);
    //    for(height = 1; height < img->height; height <<= 1);
    // Perhaps non-power-of-2 textures are acceptable?
    uint32_t width = img.width;
    uint32_t height = img.height;

    uint32_t depth = img.depth;

    if (depth <= 8)
        depth = 32;

    uint32_t format;
    switch (depth) {
    case 32:
        if (img.has_alpha())
            format = Raster::C8888;
        else{
            format = Raster::C888;
            depth = 24;
        }
        break;
    case 24:
        format = Raster::C888;
        break;
    case 16:
        format = Raster::C1555;
        break;

    case 8:
    case 4:
    default:
        THROW_OR_ABORT("Invalid raster depth");
    }

    format |= type;

    return std::make_unique<Gl3Raster>(
        width,
        height,
        depth,
        format,
        0, // compression
        1, // num_levels
        img.has_alpha(),
        raster_config);
}

std::unique_ptr<IRasterPs2> RasterFactory::create_raster_ps2(
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    uint32_t palette_size,
    uint32_t format,
    const RasterConfig& raster_config) const
{
    return std::make_unique<Ps2Raster>(width, height, depth, palette_size, format);
}

std::unique_ptr<IRasterD3d8> RasterFactory::create_raster_d3d8(
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    uint32_t palette_size,
    uint32_t format,
    uint32_t compression,
    uint32_t num_levels,
    bool has_alpha,
    const RasterConfig& raster_config) const
{
    return std::make_unique<D3d8Raster>(width, height, depth, palette_size, format, compression, num_levels, has_alpha, raster_config);
}

std::unique_ptr<IRaster> RasterFactory::make_raster_native(
    std::unique_ptr<IRaster>&& raster,
    const RasterConfig& raster_config) const
{
    if (dynamic_cast<Gl3Raster*>(raster.get()) != nullptr) {
        return raster;
    }
    auto img = raster->to_image();
    return create_raster(img, raster->type(), raster_config);
}
