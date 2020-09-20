#pragma once
#include <Mlib/Array/Array_Forward.hpp>

namespace Mlib {

Array<unsigned char> match_rgba_histograms(const Array<unsigned char>& image, const Array<unsigned char>& ref);

}
