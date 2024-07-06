#pragma once

namespace Mlib {

namespace Dff {

struct Image;
struct MipLevelMetadata;

class IRaster {
public:
    virtual ~IRaster() = default;
    virtual void from_image(const Image& image) = 0;
    virtual const MipLevelMetadata& mip_level_meta_data(uint32_t level) const = 0;
    virtual uint32_t num_levels() const = 0;
    virtual uint8_t* lock(uint32_t level, uint32_t flags) = 0;
    virtual void unlock() = 0;
};

}

}
