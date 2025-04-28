#include "Visit_Line_Segments.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Subdivided_Way_Vertex.hpp>

using namespace Mlib;

void Mlib::visit_line_segments(
    const std::list<SubdividedWayVertex>& path,
    const std::function<void(
        const SubdividedWayVertex& aL,
        const SubdividedWayVertex& aR,
        const SubdividedWayVertex& b,
        const SubdividedWayVertex& c,
        const SubdividedWayVertex& dL,
        const SubdividedWayVertex& dR,
        SegmentPosition position)>& visit)
{
    if (path.size() < 2) {
        return;
    }
    if (all(path.front().position() == path.back().position())) {
        auto a = ++path.begin();
        for (size_t i = 0; i < path.size() - 1; ++i) {
            auto b = a;
            ++b;
            if (b == path.end()) {
                b = ++path.begin();
            }
            auto c = b;
            ++c;
            if (c == path.end()) {
                c = ++path.begin();
            }
            auto d = c;
            ++d;
            if (d == path.end()) {
                d = ++path.begin();
            }
            visit(*a, *a, *b, *c, *d, *d, SegmentPosition::MIDDLE);
            ++a;
            if (a == path.end()) {
                a = ++path.begin();
            }
        }
    } else {
        auto b = path.begin();
        auto c = path.begin();
        ++c;
        auto d = c;
        ++d;
        if (d == path.end()) {
            visit(*c, *c, *b, *c, *b, *b, SegmentPosition::START | SegmentPosition::END);
        } else {
            visit(*c, *c, *b, *c, *d, *d, SegmentPosition::START);
        }
        if (path.size() == 2) {
            return;
        }
        while (true) {
            auto a = b;
            ++b;
            ++c;
            if (c == path.end()) {
                return;
            }
            ++d;
            if (d == path.end()) {
                visit(*a, *a, *b, *c, *b, *b, SegmentPosition::END);
            } else {
                visit(*a, *a, *b, *c, *d, *d, SegmentPosition::MIDDLE);
            }
        }
    }
}
