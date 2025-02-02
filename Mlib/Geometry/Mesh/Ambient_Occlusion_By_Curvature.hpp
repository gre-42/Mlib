#pragma once
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <cstddef>
#include <list>
#include <map>

namespace Mlib {

template <class TPos>
struct ColoredVertex;
template <class TDir, class TPos, size_t tndim>
class PlaneNd;
template <class TPos>
class ColoredVertexArray;

template <class TPos>
void ambient_occlusion_by_curvature(
    const std::list<FixedArray<ColoredVertex<TPos>, 3>*>& cvl,
    float strength)
{
    if (strength == 0.f) {
        return;
    }
    using PV = std::pair<OrderableFixedArray<TPos, 3>, OrderableFixedArray<float, 3>>;
    using I = funpack_t<TPos>;
    std::map<PV, std::pair<size_t, float>> curvatures;
    for (const FixedArray<ColoredVertex<TPos>, 3>* tp : cvl) {
        const FixedArray<ColoredVertex<TPos>, 3>& t = *tp;
        for (size_t i = 0; i < 3; ++i) {
            PlaneNd<float, TPos, 3> plane{t(i).normal, t(i).position};
            std::pair<size_t, float>& curvature = curvatures[PV{OrderableFixedArray{t(i).position}, OrderableFixedArray{t(i).normal}}];
            for (size_t j = 1; j < 3; ++j) {
                auto n = (TPos)dot0d(funpack(t((i + j) % 3).position), plane.normal.template casted<I>()) + plane.intercept;
                auto ta = std::sqrt(sum(squared(t((i + j) % 3).position - t(i).position)) - squared(n));
                ++curvature.first;
                curvature.second += float((I)n / (I)ta);
            }
        }
    }
    for (FixedArray<ColoredVertex<TPos>, 3>* tp : cvl) {
        for (ColoredVertex<TPos>& v : tp->flat_iterable()) {
            const auto& c = curvatures.at(PV{OrderableFixedArray{v.position}, OrderableFixedArray{v.normal}});
            auto& color3 = v.color.template row_range<0, 3>();
            color3 = round(color3.template casted<float>() * minimum(
                fixed_ones<float, 3>(),
                maximum(
                    fixed_zeros<float, 3>(),
                    1.f - strength * c.second / float(c.first)))).template casted<uint8_t>();
        }
    }
}

template <class TPos>
void ambient_occlusion_by_curvature(
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    float strength)
{
    std::list<FixedArray<ColoredVertex<TPos>, 3>*> cvl;
    for (auto& l : cvas) {
        for (auto& t : l->triangles) {
            cvl.push_back(&t);
        }
    }
    ambient_occlusion_by_curvature(cvl, strength);
}

}
