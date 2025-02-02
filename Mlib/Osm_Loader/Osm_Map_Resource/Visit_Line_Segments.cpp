#include "Visit_Line_Segments.hpp"
#include <Mlib/Array/Fixed_Array.hpp>

using namespace Mlib;

void Mlib::visit_line_segments(
    const std::list<FixedArray<CompressedScenePos, 2>>& path,
    const std::function<void(
        const FixedArray<CompressedScenePos, 2>& aL,
        const FixedArray<CompressedScenePos, 2>& aR,
        const FixedArray<CompressedScenePos, 2>& b,
        const FixedArray<CompressedScenePos, 2>& c,
        const FixedArray<CompressedScenePos, 2>& dL,
        const FixedArray<CompressedScenePos, 2>& dR,
        SegmentPosition position)>& visit)
{
    if (path.size() < 2) {
        return;
    }
    if (all(path.front() == path.back())) {
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
