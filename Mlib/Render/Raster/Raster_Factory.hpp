#pragma once
#include <Mlib/Geometry/Mesh/Load/IRaster_Factory.hpp>

namespace Mlib {

namespace Dff {

class RasterFactory: public IRasterFactory {
public:
    explicit RasterFactory();
    virtual bool is_p8_supported() const override;
    virtual std::unique_ptr<IRaster> create_raster(
        const Image& img,
        uint32_t format,
        const RasterConfig& raster_config) const override;
    virtual std::unique_ptr<IRasterD3d8> create_raster_d3d8(
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        uint32_t palette_size,
        uint32_t format,
        uint32_t compression,
        uint32_t num_levels,
        bool has_alpha,
        const RasterConfig& raster_config) const override;
    virtual std::unique_ptr<IRasterPs2> create_raster_ps2(
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        uint32_t palette_size,
        uint32_t format,
        const RasterConfig& raster_config) const override;
    virtual std::unique_ptr<IRaster> make_raster_native(
        std::unique_ptr<IRaster>&& raster,
        const RasterConfig& raster_config) const override;
};

}

}
