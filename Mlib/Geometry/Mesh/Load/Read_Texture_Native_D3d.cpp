#include "Read_Texture_Native_D3d.hpp"
#include <Mlib/Geometry/Mesh/Load/IRaster_D3d8.hpp>
#include <Mlib/Geometry/Mesh/Load/IRaster_Factory.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Dff.hpp>
#include <Mlib/Geometry/Mesh/Load/Mipmap_Level.hpp>
#include <Mlib/Geometry/Mesh/Load/Raster_Config.hpp>
#include <Mlib/Io/Cleanup.hpp>

using namespace Mlib::Dff;

std::shared_ptr<Texture> Mlib::Dff::read_texture_native_d3d8(
    std::istream& istr,
    const std::list<std::unique_ptr<IPlugin>>& plugins,
    const IRasterFactory* raster_factory,
    const RasterConfig* raster_config,
    IoVerbosity verbosity)
{
    if ((raster_factory != nullptr) && (raster_config == nullptr)) {
        THROW_OR_ABORT("Received raster factory without config");
    }
    auto texture = std::make_shared<Texture>();
    // Texture
    texture->filter_addressing = read_binary<uint32_t>(istr, "filter addressing", verbosity);
    texture->name = remove_trailing_zeros(read_string(istr, 32, "texture name", verbosity));
    texture->mask = remove_trailing_zeros(read_string(istr, 32, "texture mask", verbosity));

    if (any(verbosity & IoVerbosity::METADATA)) {
        linfo() << "Texture name: " << *texture->name << ", mask: " << *texture->mask;
    }
    if (raster_factory == nullptr) {
        return nullptr;
    }

    // Raster
    auto format = read_binary<uint32_t>(istr, "format", verbosity);
    auto has_alpha = (bool)read_binary<uint32_t>(istr, "has alpha", verbosity);
    auto width = read_binary<uint16_t>(istr, "width", verbosity);
    auto height = read_binary<uint16_t>(istr, "height", verbosity);
    auto depth = read_binary<uint8_t>(istr, "depth", verbosity);
    auto num_levels = read_binary<uint8_t>(istr, "num levels", verbosity);
    auto type = read_binary<uint8_t>(istr, "type", verbosity);
    auto compression = read_binary<uint8_t>(istr, "compression", verbosity);

    if ((format & Raster::PAL4) || (format & Raster::PAL8)) {
        if (!raster_factory->is_p8_supported()) {
            texture->raster = read_as_image(
                istr,
                width,
                height,
                depth,
                format | type,
                num_levels,
                *raster_factory,
                *raster_config,
                verbosity);
            return texture;
        }
    }

    uint32_t pal_size = palette_size(format);

    auto raster_d3d8 = raster_factory->create_raster_d3d8(
        width,
        height,
        depth,
        pal_size,
        format | type | Raster::DONTALLOCATE,
        compression,
        num_levels,
        has_alpha,
        *raster_config);

    if (pal_size != 0) {
        uint8_t* pal = raster_d3d8->palette();
        read_vector(istr, std::span{ pal, pal_size * 32 }, "D3D8 palette", verbosity);
    }
    // TODO: check if format supported and convert if necessary

    for (uint32_t i = 0; i < num_levels; i++) {
        auto size = read_binary<uint32_t>(istr, "size", verbosity);
        if (i < raster_d3d8->num_levels()) {
            auto expected_size = raster_d3d8->mipmap_level(i).size();
            if (size != expected_size) {
                THROW_OR_ABORT((std::stringstream() << "Unexpected mipmap size. Expected: " << expected_size << ", actual: " << size).str());
            }
            uint8_t* data = raster_d3d8->lock(i, Raster::LOCKWRITE | Raster::LOCKNOFETCH);
            read_vector(istr, std::span{ data, size }, "data", verbosity);
            raster_d3d8->unlock();
        } else {
            seek_relative_positive(istr, size, verbosity);
        }
    }
    if (raster_config->make_native) {
        texture->raster = raster_factory->make_raster_native(std::move(raster_d3d8), *raster_config);
    } else {
        texture->raster = std::move(raster_d3d8);
    }
    return texture;
}

std::shared_ptr<Texture> Mlib::Dff::read_texture_native_d3d9(
    std::istream& istr,
    const std::list<std::unique_ptr<IPlugin>>& plugins,
    IoVerbosity verbosity)
{
    THROW_OR_ABORT("read_texture_native_d3d9 not yet implemented");
}
