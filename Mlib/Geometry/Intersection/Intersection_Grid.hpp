#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Math/Ceil.hpp>
#include <Mlib/Math/Floor.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <functional>

namespace Mlib {

template <class TData>
class IntersectionGridPointerEntry {
public:
    explicit IntersectionGridPointerEntry(TData* data)
        : data_{ data }
    {}
    decltype(auto) aabb() const {
        return data_->aabb();
    }
    decltype(auto) entry() const {
        return *this;
    }
    template <class... TVisitors>
    bool visit(const auto& aabb, TVisitors... visitors) const {
        return data_->visit(aabb, visitors...);
    }
private:
    TData* data_;
};

template <class TPosition, size_t tndim, class TData>
class IntersectionGrid {
public:
    using TUnpackedPosition = funpack_t<TPosition>;
    IntersectionGrid(
        const AxisAlignedBoundingBox<TPosition, tndim>& boundary,
        const FixedArray<size_t, tndim>& ncells,
        const FixedArray<TPosition, tndim>& dilation_radius)
        : data_(ncells.to_array_shape())
        , boundary_{ boundary }
        , ncells_{ ncells.template casted<TUnpackedPosition>() }
        , dilation_radius_{ dilation_radius }
    {
        if (any(ncells == (size_t)0)) {
            THROW_OR_ABORT("Number of intersection grid cells cannot be zero");
        }
    }
    template <class TEntry>
    void insert(TEntry* entry) {
        insert(IntersectionGridPointerEntry{ entry });
    }
    void insert(const auto& entry) {
        auto aabb = entry.aabb();
        auto c_min = (funpack(aabb.min - boundary_.min - dilation_radius_)) / funpack(boundary_.size());
        auto c_max = (funpack(aabb.max - boundary_.min + dilation_radius_)) / funpack(boundary_.size());
        if (any(c_min < (TUnpackedPosition)0)) {
            THROW_OR_ABORT("Intersection grid entry out of lower bounds");
        }
        if (any(c_max > (TUnpackedPosition)1)) {
            THROW_OR_ABORT("Intersection grid entry out of upper bounds");
        }
        auto id_min = floor(c_min * ncells_).template casted<size_t>();
        auto id_max = minimum(
                floor(c_max * ncells_) + (TUnpackedPosition)1.f,
                ncells_ - (TUnpackedPosition)1.f).template casted<size_t>();
        if (any(id_min >= data_.template fixed_shape<tndim>())) {
            verbose_abort("Intersection grid lower index out of bounds");
        }
        if (any(id_max >= data_.template fixed_shape<tndim>())) {
            verbose_abort("Intersection grid upper index out of bounds");
        }
        AxisAlignedBoundingBox<size_t, tndim>::from_min_max(id_min, id_max).for_each_cell(
            [this, &entry](const FixedArray<size_t, tndim>& id){
                data_(id).push_back(entry.entry());
                return true;
            });
    }
    template <class... TVisitors>
    bool visit(const auto& aabb, TVisitors... visitors) const {
        auto width = aabb.size();
        if (any(width > dilation_radius_)) {
            THROW_OR_ABORT("Intersection grid: dilation radius too small");
        }
        auto center = aabb.center();
        auto c = (funpack(center - boundary_.min)) / funpack(boundary_.size());
        if (any(c < (TUnpackedPosition)0)) {
            return true;
        }
        if (any(c > (TUnpackedPosition)1)) {
            return true;
        }
        auto id = minimum(floor(c * ncells_), ncells_ - (TUnpackedPosition)1.f).template casted<size_t>();
        if (any(id >= data_.template fixed_shape<tndim>())) {
            verbose_abort("Intersection grid rounded index out of bounds");
        }
        for (const auto& child : data_(id)) {
            if (!child.visit(aabb, visitors...)) {
                return false;
            }
        }
        return true;
    }
private:
    Array<std::vector<TData>> data_;
    AxisAlignedBoundingBox<TPosition, tndim> boundary_;
    FixedArray<TUnpackedPosition, tndim> ncells_;
    FixedArray<TPosition, tndim> dilation_radius_;
};

}
