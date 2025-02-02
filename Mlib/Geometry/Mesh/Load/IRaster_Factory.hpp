#pragma once
#include <cstdint>
#include <memory>

namespace Mlib {

namespace Dff {

struct Image;
class IRaster;
struct RasterConfig;
class IRasterD3d8;
class IRasterPs2;

class IRasterFactory {
public:
    virtual bool is_p8_supported() const = 0;
    virtual std::unique_ptr<IRaster> create_raster(
        const Image& img,
        uint32_t type,
        const RasterConfig& raster_config) const = 0;
    virtual std::unique_ptr<IRasterD3d8> create_raster_d3d8(
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        uint32_t palette_size,
        uint32_t format,
        uint32_t compression,
        uint32_t num_levels,
        bool has_alpha,
        const RasterConfig& raster_config) const = 0;
    virtual std::unique_ptr<IRasterPs2> create_raster_ps2(
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        uint32_t palette_size,
        uint32_t format,
        const RasterConfig& raster_config) const = 0;
    virtual std::unique_ptr<IRaster> make_raster_native(
        std::unique_ptr<IRaster>&& raster,
        const RasterConfig& raster_config) const = 0;
};

}

}
