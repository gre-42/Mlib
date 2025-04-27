#include "Get_Region_Margin_Contour.hpp"
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Rectangle_2D.hpp>

using namespace Mlib;

std::list<FixedArray<CompressedScenePos, 2>> Mlib::get_region_margin_contour(
    const std::list<FixedArray<CompressedScenePos, 2>>& region,
    CompressedScenePos width)
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
        OsmRectangle2D rect = uninitialized;
        if (!OsmRectangle2D::from_line(
                rect,
                *a,
                *a,
                *b,
                *c,
                *d,
                *d,
                width,
                width,
                width,
                width,
                width,
                width))
        {
            THROW_OR_ABORT2(EdgeException<CompressedScenePos>(*a, *b, "Error computing region margin"));
        } else {
            result.emplace_back(rect.p11_);
        }
        ++a;
        if (a == region.end()) {
            a = region.begin();
        }
    }
    return result;
}
