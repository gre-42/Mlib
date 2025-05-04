#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Intersection_Grid.hpp>

namespace Mlib {

template <class TPosition, size_t tndim, class TPayload>
class GenericBvhGrid {
public:
    using TBvh = GenericBvh<TPosition, tndim, TPayload>;
    using Entry = IntersectionGridPointerEntry<const TBvh>;
    using Grid = IntersectionGrid<TPosition, tndim, IntersectionGridEntries<Entry, Entry>>;
    GenericBvhGrid(
        // BVH
        const FixedArray<TPosition, tndim>& max_size,
        size_t level,
        // Transition
        size_t grid_level,
        // Grid
        const FixedArray<size_t, tndim>& ncells,
        const FixedArray<TPosition, tndim>& dilation_radius)
        : root_bvh{ max_size, level }
        , grid_{ std::nullopt }
        , ncells_{ ncells }
        , dilation_radius_{ dilation_radius }
        , grid_level_{ grid_level }
    {}
    void clear() {
        grid_.reset();
        root_bvh.clear();
    }
    TBvh root_bvh;
    Grid& grid() const {
        if (!grid_.has_value()) {
            compute_grid();
        }
        return *grid_;
    }
private:
    void compute_grid() const {
        auto boundary = root_bvh.aabb();
        boundary.min -= dilation_radius_;
        boundary.max += dilation_radius_;
        grid_.emplace(boundary, ncells_, dilation_radius_);
        insert_into_grid(root_bvh, 0);
    }
    void insert_into_grid(const TBvh& b, size_t depth) const {
        if (depth == grid_level_) {
            grid_->insert(b.aabb(), IntersectionGridPointerEntry{&b}, recursive_v);
        } else {
            if (!b.data().empty()) {
                grid_->insert(b.data_aabb(), IntersectionGridPointerEntry{&b}, non_recursive_v);
            }
            for (const auto& [_, c] : b.children()) {
                insert_into_grid(c, depth + 1);
            }
        }
    }
    mutable std::optional<Grid> grid_;
    FixedArray<size_t, tndim> ncells_;
    FixedArray<TPosition, tndim> dilation_radius_;
    size_t grid_level_;
};

template <class TPosition, size_t tndim, class TPayload>
using BvhGrid = GenericBvhGrid<
    TPosition,
    tndim,
    PayloadContainer<std::list<AabbAndPayload<TPosition, tndim, TPayload>>>>;

template <class TPosition, class TCompressedPosition, class TPayload, class TCompressedPayload, size_t tndim>
using CompressedBvhGrid = GenericBvhGrid<
        TPosition,
        tndim,
        CompressedPayloadContainer<
                TPosition,
                tndim,
                std::list<AabbAndPayload<TCompressedPosition, tndim, TCompressedPayload>>,
                std::list<AabbAndPayload<TPosition, tndim, TPayload>>>>;

}
