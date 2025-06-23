#include "Get_Region_Margin_Contour.hpp"
#include <Mlib/Geometry/Exceptions/Point_Exception.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Rectangle_2D.hpp>

using namespace Mlib;

std::list<FixedArray<CompressedScenePos, 2>> Mlib::get_region_margin_contour(
    const std::list<FixedArray<CompressedScenePos, 2>>& region,
    CompressedScenePos width,
    const std::map<OrderableFixedArray<CompressedScenePos, 2>, CompressedScenePos>& garden_margin)
{
    std::list<FixedArray<CompressedScenePos, 2>> result;

    auto a = region.begin();
    for (size_t i = 0; i < region.size(); ++i) {
        auto b = a;
        ++b;
        if (b == region.end()) {
            b = region.begin();
        }
        auto c = b;
        ++c;
        if (c == region.end()) {
            c = region.begin();
        }
        auto d = c;
        ++d;
        if (d == region.end()) {
            d = region.begin();
        }
        auto a_width = garden_margin.find(OrderableFixedArray(*a));
        auto a_w = (a_width == garden_margin.end()
            ? width
            : a_width->second);
        auto b_width = garden_margin.find(OrderableFixedArray(*b));
        auto b_w = (b_width == garden_margin.end()
            ? width
            : b_width->second);
        auto c_width = garden_margin.find(OrderableFixedArray(*c));
        auto c_w = (c_width == garden_margin.end()
            ? width
            : c_width->second);

        OsmRectangle2D rect = uninitialized;
        if (!OsmRectangle2D::from_line(
                rect,
                *a,
                *a,
                *b,
                *c,
                *d,
                *d,
                2.f * std::min(a_w, b_w),
                2.f * std::min(a_w, b_w),
                2.f * std::min(b_w, c_w),
                2.f * std::min(b_w, c_w),
                2.f * width,
                2.f * width))
        {
            using PE = PointException<CompressedScenePos, 2>;
            THROW_OR_ABORT2(PE(*b, "Error computing region margin"));
        } else {
            result.emplace_back(rect.p01_);
        }
        ++a;
        if (a == region.end()) {
            a = region.begin();
        }
    }
    return result;
}
