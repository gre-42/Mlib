#pragma once
#include <Mlib/Geometry/Mesh/Load/IRaster_D3d8.hpp>
#include <Mlib/Geometry/Mesh/Load/Mipmap_Level.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <vector>

namespace Mlib {

namespace Dff {

struct RasterConfig;

struct Image;

class D3d8Raster : public IRasterD3d8 {
public:
    D3d8Raster(
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        uint32_t palette_size,
        uint32_t format,
        uint32_t compression,
        uint32_t num_levels,
        bool has_alpha,
        const RasterConfig& cfg);
    virtual ~D3d8Raster() override;
    virtual void from_image(const Image& image) override;
    virtual Image to_image() override;
    virtual uint32_t width() const override;
    virtual uint32_t height() const override;
    virtual uint32_t type() const override;
    virtual const MipmapLevel& mipmap_level(uint32_t level) const override;
    virtual uint32_t num_levels() const override;
    virtual uint8_t* lock(uint32_t level, uint32_t lock_mode) override;
    virtual void unlock() override;
    virtual uint8_t* palette() override;
    virtual std::shared_ptr<ITextureHandle> texture_handle() override;
private:
    void allocate_dxt(uint32_t dxt);
    void compute_mip_level_metadata();
    void set_format();
    uint32_t flags() const;
    uint32_t format() const;
    uint32_t format_;
    uint32_t num_levels_;
    uint32_t width_;
    uint32_t height_;
    uint32_t depth_;
    uint32_t stride_;
    bool native_has_alpha_;
    uint32_t native_bpp_;
    bool native_autogen_mipmap_;
    uint32_t native_internal_format_;
    GLenum native_format_;
    uint32_t private_flags_;
    uint32_t custom_format_;
    uint8_t* pixels_;
    std::vector<MipmapLevel> levels_;
    std::vector<uint8_t> palette_;
};

}

}
