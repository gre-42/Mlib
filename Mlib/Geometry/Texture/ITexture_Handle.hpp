#pragma once
#include <Mlib/Memory/Integral_Cast.hpp>
#include <cstdint>
#include <type_traits>

namespace Mlib {

enum class MipmapMode;
enum class ColorMode;

class ITextureHandle {
public:
    virtual ~ITextureHandle() = default;
    template <class T>
    inline T handle() const {
        if constexpr (std::is_same_v<T, uint32_t>) {
            return handle32();
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return handle64();
        } else {
            THROW_OR_ABORT("Unsupported texture handle type");
        }
    }
    template <class T>
    inline T& handle() {
        if constexpr (std::is_same_v<T, uint32_t>) {
            return handle32();
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return handle64();
        } else {
            THROW_OR_ABORT("Unsupported texture handle type");
        }
    }
    virtual uint32_t handle32() const = 0;
    virtual uint64_t handle64() const = 0;
    virtual uint32_t& handle32() = 0;
    virtual uint64_t& handle64() = 0;
    virtual bool texture_is_loaded_and_try_preload() = 0;
    virtual ColorMode color_mode() const = 0;
    virtual MipmapMode mipmap_mode() const = 0;
};

}
