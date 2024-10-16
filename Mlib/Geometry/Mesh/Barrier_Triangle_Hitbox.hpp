#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <cstddef>
#include <cstdint>
#include <list>
#include <memory>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
enum class PhysicsMaterial: uint32_t;
template <typename TData, size_t... tshape>
class FixedArray;

template <class TPos>
UUVector<FixedArray<TPos, 3, 3>> barrier_triangle_hitbox(
    const FixedArray<TPos, 3>& am,
    const FixedArray<TPos, 3>& bm,
    const FixedArray<TPos, 3>& cm,
    const FixedArray<float, 3>& half_width_a,
    const FixedArray<float, 3>& half_width_b,
    const FixedArray<float, 3>& half_width_c,
    bool ab_is_contour_edge,
    bool bc_is_contour_edge,
    bool ca_is_contour_edge);

template <class TPos>
std::vector<std::shared_ptr<ColoredVertexArray<TPos>>> create_barrier_triangle_hitboxes(
    const ColoredVertexArray<TPos>& cva,
    float half_width,
    PhysicsMaterial destination_physics_material);

}
