#include "Mipmap_Level.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <list>

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3

namespace Mlib {
namespace Dff {

std::vector<MipmapLevel> MipmapLevel::compute(
    uint32_t width,
    uint32_t height,
    uint32_t stride,
    uint32_t internal_format,
    uint32_t max_num_levels,
    AllocationMode allocation_mode)
{
    uint32_t min_dim = 1;

    switch (internal_format){
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        min_dim = 4;
        break;
    }

    std::list<MipmapLevel> levels;
    auto append_level = [&](){
        auto& m = levels.emplace_back(MipmapLevel{.width = width, .height = height, .stride = stride});
        if (allocation_mode == AllocationMode::ALLOCATE) {
            m.data.resize(m.size());
        }
        };
    if (max_num_levels == 0) {
        THROW_OR_ABORT("Number of mipmap levels cannot be zero");
    }
    append_level();
    for (uint32_t i = 0; ((width > min_dim) || (height > min_dim)) && (i < max_num_levels); i++) {
        if (width > min_dim) {
            width /= 2;
            stride /= 2;
        }
        if (height > min_dim) {
            height /= 2;
        }
        append_level();
    }
    return { levels.begin(), levels.end() };
}

}
}
