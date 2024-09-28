#pragma once
#include <cstdint>
#include <memory>

namespace Mlib {

class ITextureHandle;
    
namespace Dff {

struct Image;
struct MipmapLevel;

class IRaster {
public:
    virtual ~IRaster() = default;
    virtual void from_image(const Image& image) = 0;
    virtual Image to_image() = 0;
    virtual uint32_t width() const = 0;
    virtual uint32_t height() const = 0;
    virtual const MipmapLevel& mipmap_level(uint32_t level) const = 0;
    virtual uint32_t num_levels() const = 0;
    virtual uint32_t type() const = 0;
    virtual uint8_t* lock(uint32_t level, uint32_t flags) = 0;
    virtual void unlock() = 0;
    virtual std::shared_ptr<ITextureHandle> texture_handle() = 0;
};

}

}
