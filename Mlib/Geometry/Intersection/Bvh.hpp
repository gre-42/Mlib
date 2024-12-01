#pragma once
#include <Mlib/Geometry/Intersection/Bvh_Fwd.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Pow.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>
#include <iomanip>
#include <list>
#include <ostream>
#include <type_traits>
#include <vector>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

template <class TSize>
class Svg;

struct BvhPrintingOptions {
    bool level = true;
    bool aabb = true;
    bool data = true;
    bool children = true;
};

enum class BvhDataRadiusType {
    ZERO,
    NONZERO
};

/**
 * Bounding volume hierarchy
 */
template <class TPosition, size_t tndim, class TData, AabbExtensionDirection extension_direction>
class GenericBvh {
public:
    explicit GenericBvh(const FixedArray<TPosition, tndim>& max_size, size_t level)
        : max_size_{max_size}
        , level_{level}
    {}

    auto* insert(const auto& aabb, const auto& payload) {
        return insert(AabbAndPayload{aabb, payload});
    }

    auto* insert(const auto& entry) {
        auto max_size_children = max_size_ * (1 << (level_ - 1));
        if ((level_ == 0) || any(entry.aabb().size() > max_size_children)) {
            return &data_.add(entry).payload();
        }
        for (auto& c : children_) {
            if ((extension_direction == AabbExtensionDirection::POSITIVE) &&
                any(entry.aabb().min < c.first.min))
            {
                continue;
            }
            AxisAlignedBoundingBox<TPosition, tndim> bb = c.first;
            bb.extend(entry.aabb());
            // if (all(bb.size() <= TPosition(level_) * max_size_)) {
            if (all(bb.size() <= max_size_children)) {
                c.first = bb;
                return c.second.insert(entry);
            }
        }
        return children_.emplace_back(entry.aabb(), GenericBvh{max_size_, level_ - 1}).second.insert(entry);
    }

    void clear() {
        data_.clear();
        children_.clear();
    }

    size_t size() const {
        size_t result = data_.size();
        for (const auto& [_, c] : children_) {
            result += c.size();
        }
        return result;
    }

    size_t layer_size() const {
        return data_.size() + children_.size();
    }

    bool empty() const {
        return data_.empty() && children_.empty();
    }

    template <class... TVisitors>
    bool visit(const auto& aabb, TVisitors... visitors) const {
        if (!data_.visit(aabb, visitors...)) {
            return false;
        }
        for (const auto& c : children_) {
            if (aabb.intersects(c.first)) {
                if (!c.second.visit(aabb, visitors...)) {
                    return false;
                }
            }
        }
        return true;
    }

    template <class... TVisitors>
    bool visit_pairs(const auto& aabb, TVisitors... visitors) const {
        if (!data_.visit_pairs(aabb, visitors...)) {
            return false;
        }
        for (const auto& c : children_) {
            if (aabb.intersects(c.first)) {
                if (!c.second.visit_pairs(aabb, visitors...)) {
                    return false;
                }
            }
        }
        return true;
    }

    AxisAlignedBoundingBox<TPosition, tndim> aabb() const {
        auto result = AxisAlignedBoundingBox<TPosition, tndim>::empty();
        data_.visit_all([&](const auto& d){
            result.extend(d.aabb());
            return true;
        });
        for (const auto& c : children_) {
            result.extend(c.first);
        }
        return result;
    }

    void print(std::ostream& ostr, const BvhPrintingOptions& opts, size_t rec = 0) const {
        std::string indent(rec, ' ');
        if (opts.level) {
            ostr << indent << "level " << level_ << std::endl;
        }
        if (opts.data) {
            ostr << indent << "data " << data_.size() << std::endl;
            if (opts.aabb) {
                data_.print(ostr, rec);
            }
        }
        if (opts.children) {
            ostr << indent << "children " << children_.size() << std::endl;
            for (const auto& c : children_) {
                if (opts.aabb) {
                    c.first.print(ostr, rec + 1);
                }
                c.second.print(ostr, opts, rec + 1);
            }
        }
    }

    float search_time(BvhDataRadiusType data_radius_type) const {
        float res = (float)children_.size();
        for (const auto& c : children_) {
            if (data_radius_type == BvhDataRadiusType::ZERO) {
                // The following assumptions are made:
                // 1. The query radius is rather small.
                // 2. The children do not overlap.
                res += c.second.search_time(data_radius_type) / (float)children_.size();
            } else if (data_radius_type == BvhDataRadiusType::NONZERO) {
                // The following assumptions are made:
                // 1. The query radius is zero.
                // 2. The centers of the children are typical query points.
                // Under these assumptions, compute the probability of having
                // to traverse the child "c".
                size_t nintersections = 0;
                for (const auto& other : children_) {
                    if (c.first.contains(other.first.center())) {
                        ++nintersections;
                    }
                }
                res += c.second.search_time(data_radius_type) * (float)nintersections / (float)children_.size();
            }
        }
        res += (float)data_.size();
        return res;
    }

    template <class... TVisitors>
    bool visit_all(TVisitors... visitors) const {
        if (!data_.visit_all(visitors...)) {
            return false;
        }
        for (const auto& c : children_) {
            if (!c.second.visit_all(visitors...)) {
                return false;
            }
        }
        return true;
    }

    void modify_bottom_up(
        const auto &node_visitor,
        const auto &leaf_visitor)
    {
        for (auto &[_, c] : children_) {
            c.modify_bottom_up(node_visitor, leaf_visitor);
        }
        node_visitor(children_);
        leaf_visitor(data_);
    }

    bool visit_bvhs(const auto &visitor) const {
        if (!visitor(*this)) {
            return false;
        }
        for (const auto &c : children_) {
            if (!c.second.visit_bvhs(visitor)) {
                return false;
            }
        }
        return true;
    }

    template <class TPayload = void>
    TPosition min_distance(
        const FixedArray<TPosition, tndim>& p,
        const TPosition& max_distance,
        const auto& compute_distance,
        const TPayload** nearest_payload = nullptr) const
    {
        TPosition min_distance = INFINITY;
        visit(AxisAlignedBoundingBox<TPosition, tndim>::from_center_and_radius(p, max_distance),
            [&min_distance, &compute_distance, nearest_payload](const auto& payload)
        {
            TPosition dist = compute_distance(payload);
            if (dist < min_distance) {
                min_distance = dist;
                if (nearest_payload != nullptr) {
                    *nearest_payload = &payload;
                }
            }
            return true;
        });
        return min_distance;
    }

    template <class TPayload>
    std::vector<std::pair<TPosition, const TPayload*>> min_distances(
        size_t k,
        const FixedArray<TPosition, tndim>& p,
        const TPosition& max_distance,
        const auto& compute_distance) const
    {
        std::vector<std::pair<TPosition, const TPayload*>> result(k);
        std::fill(result.begin(), result.end(), std::make_pair(INFINITY, nullptr));
        auto predicate = [](const auto& a, const auto& b){return a.first < b.first;};
        visit(AxisAlignedBoundingBox<TPosition, tndim>::from_center_and_radius(p, max_distance),
            [&result, &compute_distance, &predicate](const TPayload& payload)
        {
            TPosition dist = compute_distance(payload);
            if (dist < result.back().first) {
                result.resize(result.size() - 1);
                // From: https://stackoverflow.com/questions/15843525/how-do-you-insert-the-value-in-a-sorted-vector
                result.insert( 
                    std::upper_bound(
                        result.begin(),
                        result.end(),
                        std::make_pair(dist, &payload),
                        predicate),
                    std::make_pair(dist, &payload));
            }
            return true;
        });
        auto last = std::lower_bound(result.begin(), result.end(), std::make_pair(INFINITY, nullptr), predicate);
        result.resize(size_t(last - result.begin()));
        return result;
    }

    bool has_neighbor(
        const FixedArray<TPosition, tndim>& p,
        const TPosition& max_distance,
        const auto& compute_distance) const
    {
        return !visit(
            AxisAlignedBoundingBox<TPosition, tndim>::from_center_and_radius(p, max_distance),
            [&max_distance, &compute_distance](const auto& payload) {
                return compute_distance(payload) > max_distance;
            });
    }

    bool has_neighbor2(
        const FixedArray<TPosition, tndim>& p,
        const TPosition& max_distance,
        const auto& compute_distance_squared) const
    {
        return !visit(
            AxisAlignedBoundingBox<TPosition, tndim>::from_center_and_radius(p, max_distance),
            [max_distance2=squared(max_distance), &compute_distance_squared]
            (const auto& payload) {
                return compute_distance_squared(payload) > max_distance2;
            });
    }

    GenericBvh repackaged(const FixedArray<TPosition, tndim>& max_size, size_t level) const
    {
        GenericBvh result{ max_size, level };
        fill(result);
        return result;
    }

    void fill(GenericBvh& other) const
    {
        data_.fill(other);
        for (const auto& c : children_) {
            c.second.fill(other);
        }
    }

    void optimize_search_time(BvhDataRadiusType data_radius_type, std::ostream& ostr) const {
        if (empty()) {
            return;
        }
        size_t sz = size();
        for (float max_size_fac = 0.5f; max_size_fac < 10.f; max_size_fac *= 2.f) {
            for (size_t level = 0; level < 20; ++level) {
                std::cout << "Max size fac: " << std::setw(5) << max_size_fac;
                std::cout << " Level: " << std::setw(5) << level;
                std::cout << " Speedup: " <<
                    std::setw(10) <<
                    ((float)sz / repackaged(max_size_ * max_size_fac, level).search_time(data_radius_type) - 1) <<
                    std::endl;
            }
        }
    }

    template <class TSize>
    void plot_bvh(
        const FixedArray<TPosition, tndim>& origin,
        Svg<TSize>& svg,
        size_t axis0,
        size_t axis1,
        const TPosition& stroke_width = (TPosition)0.05) const
    {
        static_assert(tndim >= 2);
        auto plot_aabb_raw = [&](const AxisAlignedBoundingBox<TPosition, tndim>& aabb) {
            FixedArray<TPosition, tndim> a = aabb.min - origin;
            FixedArray<TPosition, tndim> b = aabb.max - origin;
            // svg.template draw_path<TPosition>(
            //     { a(axis0), b(axis0), b(axis0), a(axis0), a(axis0) },
            //     { a(axis1), a(axis1), b(axis1), b(axis1), a(axis1) },
            //     stroke_width);
            svg.template draw_rectangle<TPosition>(
                a(axis0), a(axis1), b(axis0), b(axis1),
                stroke_width);
        };
        auto plot_aabb = [&](const AxisAlignedBoundingBox<TPosition, tndim>& aabb) {
            if (all(aabb.min == aabb.max)) {
                plot_aabb_raw(AxisAlignedBoundingBox<TPosition, tndim>::from_center_and_radius(aabb.min, stroke_width));
            } else {
                plot_aabb_raw(aabb);
            }
        };
        data_.visit_all([&](const auto& d){
            plot_aabb(d.aabb());
            return true;
        });
        for (const auto& child : children_) {
            plot_aabb(child.first);
            child.second.plot_bvh(origin, svg, axis0, axis1, stroke_width);
        }
    }

    template <class TSize>
    void plot_svg(const std::string& filename, size_t axis0, size_t axis1) const {
        std::ofstream ofs{ filename };
        auto sz = aabb().template casted<TSize>().size();
        Svg<TSize> svg{ ofs, sz(axis0), sz(axis1) };
        plot_bvh(aabb().min, svg, axis0, axis1);
        svg.finish();
        ofs.flush();
        if (ofs.fail()) {
            THROW_OR_ABORT("Could not write to file \"" + filename + '"');
        }
    }

    const TData& data() const
    {
        return data_;
    }

    const std::list<std::pair<AxisAlignedBoundingBox<TPosition, tndim>, GenericBvh>>& children() const
    {
        return children_;
    }
private:
    FixedArray<TPosition, tndim> max_size_;
    size_t level_;
    TData data_;
    std::list<std::pair<AxisAlignedBoundingBox<TPosition, tndim>, GenericBvh>> children_;
};

template <class TPosition, size_t tndim, class TData, AabbExtensionDirection extension_direction>
std::ostream& operator << (std::ostream& ostr, const GenericBvh<TPosition, tndim, TData, extension_direction>& bvh) {
    bvh.print(ostr, BvhPrintingOptions{});
    return ostr;
}

template <class TPosition, size_t tndim, class TPayload>
class AabbAndPayload {
public:
    AabbAndPayload(
        const AxisAlignedBoundingBox<TPosition, tndim>& aabb,
        const TPayload& payload)
        : aabb_{ aabb }
        , payload_{ payload }
    {}
    inline const auto& aabb() const {
        return aabb_;
    }
    inline const auto& payload() const {
        return payload_;
    }
    inline auto& payload() {
        return payload_;
    }
private:
    AxisAlignedBoundingBox<TPosition, tndim> aabb_;
    TPayload payload_;
};

template <class TPosition, size_t tndim>
class PointWithoutPayload {
public:
    PointWithoutPayload(const FixedArray<TPosition, tndim>& point)
        : point_{ point }
    {}
    inline auto aabb() const {
        return AxisAlignedBoundingBox<TPosition, tndim>::from_point(point_);
    }
    inline const auto& payload() const {
        return point_;
    }
private:
    FixedArray<TPosition, tndim> point_;
};

template <class TContainer>
class PayloadContainer {
public:
    template <class... TArgs>
    auto& add(TArgs... args) {
        return data_.emplace_back(args...);
    }
    void fill(auto& container) const {
        for (const auto& d : data_) {
            container.insert(d);
        }
    }
    bool visit(const auto& aabb, const auto& visitor) const {
        for (const auto& d : data_) {
            if (aabb.intersects(d.aabb())) {
                if (!visitor(d.payload())) {
                    return false;
                }
            }
        }
        return true;
    }
    bool visit_pairs(const auto& aabb, const auto& visitor) const {
        for (const auto& d : data_) {
            if (aabb.intersects(d.aabb())) {
                if (!visitor(d)) {
                    return false;
                }
            }
        }
        return true;
    }
    bool visit_all(const auto& visitor) const {
        for (const auto& d : data_) {
            if (!visitor(d)) {
                return false;
            }
        }
        return true;
    }
    void print(std::ostream& ostr, size_t rec = 0) const {
        for (const auto& d : data_) {
            d.aabb().print(ostr, rec + 1);
        }
    }
    bool empty() const {
        return data_.empty();
    }
    size_t size() const {
        return data_.size();
    }
    void clear() {
        data_.clear();
    }
private:
    TContainer data_;
};

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
