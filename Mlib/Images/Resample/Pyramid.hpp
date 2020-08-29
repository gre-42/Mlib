#include <Mlib/Array/Array.hpp>
#include <deque>
#include <functional>

namespace Mlib {

Array<float> down_sample(
    const Array<float>& image,
    const ArrayShape& reduction);

Array<float> down_sample2(const Array<float>& image);

Array<float> up_sample2(const Array<float>& image);

void resampling_pyramid(
    const Array<float>& images,
    size_t nlevels,
    size_t reduction,
    const std::function<void(const Array<float>&)>& operation);

Array<bool> multi_scale_harris(
    const Array<float>& image,
    size_t nlevels,
    float gamma = 1.5);

}
