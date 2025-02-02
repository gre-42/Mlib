#pragma once
#include <cstdint>

namespace Mlib {
namespace Dff {

class Palette {
public:
    inline Palette(Uninitialized) {}
    inline uint8_t& operator [](uint32_t i) {
        return palette_[i];
    }
    inline const uint8_t& operator [](uint32_t i) const {
        return palette_[i];
    }
    inline void resize(uint32_t new_size) {
        size_ = new_size;
    }
    inline bool empty() const {
        return size_ == 0;
    }
    inline uint32_t size() const {
        return size_;
    }
    inline const uint8_t* data() const {
        return palette_;
    }
    inline uint8_t* data() {
        return palette_;
    }
private:
    uint8_t palette_[4 * 256];
    uint32_t size_;
};

}
}
