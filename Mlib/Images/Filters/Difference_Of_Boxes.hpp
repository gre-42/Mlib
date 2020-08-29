#pragma once
#include <Mlib/Images/Filters/Small_Box_Filter.hpp>


namespace Mlib {

template <class TData>
Array<TData> difference_of_boxes(const Array<TData>& image, const TData& boundary_value)
{
    ArrayShape radiuses(image.ndim());
    radiuses = 1;
    return small_box_filter_NWE(image, radiuses, boundary_value) - image;
}

}
