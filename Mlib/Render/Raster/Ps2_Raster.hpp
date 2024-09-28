#pragma once
#include <Mlib/Geometry/Mesh/Load/IRaster.hpp>
#include <Mlib/Geometry/Mesh/Load/IRaster_Ps2.hpp>
#include <Mlib/Geometry/Mesh/Load/Mipmap_Level.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <vector>

namespace Mlib {

namespace Dff {

struct Image;

class Ps2Raster : public IRasterPs2 {
public:
    Ps2Raster(
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        uint32_t palette_size,
        uint32_t format);
    virtual ~Ps2Raster() override;
    virtual void from_image(const Image& image) override;
    virtual Image to_image() override;
    virtual uint32_t width() const override;
    virtual uint32_t height() const override;
    virtual const MipmapLevel& mipmap_level(uint32_t level) const override;
    virtual uint32_t num_levels() const override;
    virtual uint32_t& flags() override;
    virtual uint32_t type() const override;
    virtual uint32_t pixel_size() const override;
    virtual uint8_t* lock(uint32_t level, uint32_t lock_mode) override;
    virtual void unlock() override;
    virtual uint8_t* lock_palette(uint32_t lock_mode) override;
    virtual void unlock_palette() override;
    virtual std::shared_ptr<ITextureHandle> texture_handle() override;
private:
    void create_texture();
    void unswizzle_raster();
    void swizzle_raster();
    void convert_palette();
    // void calcTEX1(uint64_t *tex1, int32_t filter);
    uint32_t locked_level_;
    uint32_t width_;
    uint32_t height_;
    uint32_t depth_;
    uint32_t format_;
    uint32_t type_;
    uint32_t flags_;
    uint32_t stride_;
    uint32_t pixel_size_;
    uint8_t* pixels_;
    uint32_t total_size_;
    uint32_t native_flags_;
    uint32_t private_flags_;
    std::vector<uint8_t> pixel_data_;
    std::vector<uint8_t> palette_;
    std::vector<MipmapLevel> levels_;
};

}

}
