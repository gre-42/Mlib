#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Math/Ceil.hpp>
#include <Mlib/Math/Floor.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <functional>
#include <sstream>
#include <vector>

namespace Mlib {

static struct Recursive {} recursive_v;
static struct NonRecursive {} non_recursive_v;

template <class TData>
class IntersectionGridPointerEntry {
public:
    explicit IntersectionGridPointerEntry(TData* data)
        : data_{ data }
    {}
    template <class... TVisitors>
    bool visit_data(const auto& aabb, TVisitors... visitors) const {
        return data_->visit_data(aabb, visitors...);
    }
    template <class... TVisitors>
    bool visit(const auto& aabb, TVisitors... visitors) const {
        return data_->visit(aabb, visitors...);
    }
private:
    TData* data_;
};

template <class TNonRecursive, class TRecursive>
class IntersectionGridEntries {
public:
    template <class... TVisitors>
    bool visit(const auto& aabb, TVisitors... visitors) const {
        for (const auto& child : non_recursive_) {
            if (!child.visit_data(aabb, visitors...)) {
                return false;
            }
        }
        for (const auto& child : recursive_) {
            if (!child.visit(aabb, visitors...)) {
                return false;
            }
        }
        return true;
    }
    void push_back(TNonRecursive data0, NonRecursive) {
        non_recursive_.emplace_back(std::move(data0));
    }
    void push_back(TRecursive data1, Recursive) {
        recursive_.emplace_back(std::move(data1));
    }
private:
    std::vector<TNonRecursive> non_recursive_;
    std::vector<TRecursive> recursive_;
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
    template <class... Args>
    void insert(const auto& aabb, const auto& entry, Args... args) {
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
            [this, &entry, &args...](const FixedArray<size_t, tndim>& id){
                data_(id).push_back(entry, std::forward<Args>(args)...);
                return true;
            });
    }
    template <class... TVisitors>
    bool visit(const auto& aabb, TVisitors... visitors) const {
        if (auto half_width = aabb.size() / 2; any(half_width > dilation_radius_)) {
            std::stringstream sstr;
            sstr <<
                "Intersection grid: Dilation radius too small. Half width: " <<
                half_width << ". Dilation radius: " << dilation_radius_;
            THROW_OR_ABORT(sstr.str());
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
        return data_(id).visit(aabb, visitors...);
    }
private:
    Array<TData> data_;
    AxisAlignedBoundingBox<TPosition, tndim> boundary_;
    FixedArray<TUnpackedPosition, tndim> ncells_;
    FixedArray<TPosition, tndim> dilation_radius_;
};

}
