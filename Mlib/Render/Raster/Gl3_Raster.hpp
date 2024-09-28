#pragma once
#include <Mlib/Geometry/Mesh/Load/IRaster.hpp>
#include <Mlib/Geometry/Mesh/Load/Mipmap_Level.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace Mlib {

namespace Dff {

struct RasterConfig;

class Gl3Raster : public IRaster {
public:
    Gl3Raster(
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        uint32_t format,
        uint32_t compression,
        uint32_t num_levels,
        bool has_alpha,
        const RasterConfig& cfg);
    virtual ~Gl3Raster() override;
    virtual void from_image(const Image& image) override;
    virtual Image to_image() override;
    virtual uint32_t width() const override;
    virtual uint32_t height() const override;
    virtual uint32_t type() const override;
    virtual const MipmapLevel& mipmap_level(uint32_t level) const override;
    virtual uint32_t num_levels() const override;
    virtual uint8_t* lock(uint32_t level, uint32_t lock_mode) override;
    virtual void unlock() override;
    virtual std::shared_ptr<ITextureHandle> texture_handle() override;
private:
    void allocate_dxt(const RasterConfig& cfg);
    void create_texture();
    uint32_t flags() const;
    uint32_t format() const;
    uint32_t num_levels_;
    uint32_t format_;
    uint32_t compression_;
    bool has_alpha_;
    uint32_t native_internal_format_;
    uint32_t width_;
    uint32_t height_;
    uint32_t depth_;
    uint32_t stride_;
    uint32_t native_type_;
    bool native_has_alpha_;
    uint32_t native_bpp_;
    bool native_is_compressed_;
    uint32_t native_num_levels_;
    bool native_autogen_mipmap_;
    std::shared_ptr<ITextureHandle> native_texture_id_;
    GLenum native_format_;
    std::vector<MipmapLevel> levels_;
    uint32_t private_flags_;
    std::vector<uint8_t> pixels_;
    std::optional<uint32_t> locked_level_;
    bool flip_y_axis_;
};

}

}
