#include <cstddef>

namespace Mlib {

class StbImage4;

StbImage4 extrapolate_rgba_colors(const StbImage4& img, float sigma, size_t niterations);

}
