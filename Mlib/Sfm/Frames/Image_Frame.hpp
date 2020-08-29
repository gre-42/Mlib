#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib { namespace Sfm {

class ImageFrame {
public:
    ImageFrame();
    ImageFrame& operator = (float rhs);
    void load_from_file(const std::string& filename);
    void save_to_file(const std::string& filename) const;
    void save_axis_to_file(const std::string& filename, size_t axis, float min, float max) const;
    Array<float> grayscale_x3() const;
    Array<float> grayscale;
    Array<float> rgb;
    Array<bool> mask;
private:
    mutable Array<float> grayscale_x3_;
};

}}
