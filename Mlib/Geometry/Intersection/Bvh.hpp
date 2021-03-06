#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <iomanip>
#include <list>
#include <ostream>
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

/**
 * Bounding volume hierarchy
 */
template <class TData, class TPayload, size_t tndim>
class Bvh {
public:
    explicit Bvh(const FixedArray<TData, tndim>& max_size, size_t level)
    : max_size_{max_size},
      level_{level}
    {}

    const TPayload* insert(
        const AxisAlignedBoundingBox<TData, tndim>& aabb,
        const TPayload& data)
    {
        if (level_ == 0) {
            data_.push_back({aabb, data});
            return &data_.back().second;
        }
        for (auto& c : children_) {
            AxisAlignedBoundingBox<TData, tndim> bb = c.first;
            bb.extend(aabb);
            // if (all(bb.size() <= TData(level_) * max_size_)) {
            if (all(bb.size() <= max_size_ * std::pow(TData(2), TData(level_)))) {
                c.first = bb;
                return c.second.insert(aabb, data);
            }
        }
        children_.push_back({aabb, Bvh{max_size_, level_ - 1}});
        return children_.back().second.insert(aabb, data);
    }

    template <class TVisitor>
    bool visit(const AxisAlignedBoundingBox<TData, tndim>& aabb, const TVisitor& visitor) const {
        for (const auto& d : data_) {
            if (d.first.intersects(aabb)) {
                if (!visitor(d.second)) {
                    return false;
                }
            }
        }
        for (const auto& c : children_) {
            if (c.first.intersects(aabb)) {
                if (!c.second.visit(aabb, visitor)) {
                    return false;
                }
            }
        }
        return true;
    }

    AxisAlignedBoundingBox<TData, tndim> aabb() const {
        AxisAlignedBoundingBox<TData, tndim> result;
        for (const auto& d : data_) {
            result.extend(d.first);
        }
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
            for (const auto& d : data_) {
                if (opts.aabb) {
                    d.first.print(ostr, rec + 1);
                }
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

    float search_time() const {
        float res = (float)children_.size();
        for (const auto& c : children_) {
            res += c.second.search_time() / children_.size();
        }
        res += data_.size();
        return res;
    }

    template <class TVisitor>
    bool visit_all(const TVisitor& visitor) const {
        for (const auto& x : data_) {
            if (!visitor(x)) {
                return false;
            }
        }
        for (const auto& c : children_) {
            if (!c.second.visit_all(visitor)) {
                return false;
            }
        }
        return true;
    }

    template <class TComputeDistance>
    TData min_distance(
        const FixedArray<TData, tndim>& p,
        const TData& max_distance,
        const TComputeDistance& compute_distance,
        const TPayload** nearest_payload = nullptr) const
    {
        TData min_distance = INFINITY;
        visit(AxisAlignedBoundingBox<TData, tndim>(p, max_distance),
            [&min_distance, &compute_distance, nearest_payload](const TPayload& playload)
        {
            TData dist = compute_distance(playload);
            if (dist < min_distance) {
                min_distance = dist;
                if (nearest_payload != nullptr) {
                    *nearest_payload = &playload;
                }
            }
            return true;
        });
        return min_distance;
    }

    template <class TComputeDistance>
    std::vector<std::pair<TData, const TPayload*>> min_distances(
        size_t k,
        const FixedArray<TData, tndim>& p,
        const TData& max_distance,
        const TComputeDistance& compute_distance) const
    {
        std::vector<std::pair<TData, const TPayload*>> result(k);
        std::fill(result.begin(), result.end(), std::make_pair(INFINITY, nullptr));
        auto predicate = [](const auto& a, const auto& b){return a.first < b.first;};
        visit(AxisAlignedBoundingBox<TData, tndim>(p, max_distance),
            [&result, &compute_distance, &predicate](const TPayload& playload)
        {
            TData dist = compute_distance(playload);
            if (dist < result.back().first) {
                result.resize(result.size() - 1);
                // From: https://stackoverflow.com/questions/15843525/how-do-you-insert-the-value-in-a-sorted-vector
                result.insert( 
                    std::upper_bound(
                        result.begin(),
                        result.end(),
                        std::make_pair(dist, &playload),
                        predicate),
                    std::make_pair(dist, &playload));
            }
            return true;
        });
        auto last = std::lower_bound(result.begin(), result.end(), std::make_pair(INFINITY, nullptr), predicate);
        result.resize(last - result.begin());
        return result;
    }

    template <class TComputeDistance>
    bool has_neighbor(
        const FixedArray<TData, tndim>& p,
        const TData& max_distance,
        const TComputeDistance& compute_distance) const
    {
        return !visit(AxisAlignedBoundingBox<TData, tndim>(p, max_distance), [&max_distance, &compute_distance](const TPayload& playload) {
            return compute_distance(playload) > max_distance;
        });
    }

    Bvh repackaged(const FixedArray<TData, tndim>& max_size, size_t level) const
    {
        Bvh<TData, TPayload, tndim> result{ max_size, level };
        visit_all([&](const auto& d) { result.insert(d.first, d.second); return true; });
        return result;
    }

    void optimize_search_time(std::ostream& ostr) const {
        for (TData max_size_fac = (TData)0.5; max_size_fac < 10; max_size_fac *= 2) {
            for (size_t level = 5; level < 20; ++level) {
                std::cout << "Max size fac: " << std::setw(5) << max_size_fac;
                std::cout << " Level: " << std::setw(5) << level;
                std::cout << " Search time: " <<
                    std::setw(10) <<
                    repackaged(max_size_fac * max_size_, level).search_time() <<
                    std::endl;
            }
        }
    }

    template <class TSize>
    void plot_bvh(const FixedArray<TData, tndim>& origin, Svg<TSize>& svg, size_t axis0, size_t axis1) const {
        static_assert(tndim >= 2);
        auto plot_aabb = [&](const AxisAlignedBoundingBox<TData, tndim>& aabb) {
            FixedArray<TData, tndim> a = aabb.min() - origin;
            FixedArray<TData, tndim> b = aabb.max() - origin;
            // svg.template draw_path<TData>(
            //     { a(axis0), b(axis0), b(axis0), a(axis0), a(axis0) },
            //     { a(axis1), a(axis1), b(axis1), b(axis1), a(axis1) },
            //     (TData)0.05);
            svg.template draw_rectangle<TData>(
                a(axis0), a(axis1), b(axis0), b(axis1),
                (TData)0.05);
        };
        for (const std::pair<AxisAlignedBoundingBox<TData, tndim>, TPayload>& d : data_) {
            plot_aabb(d.first);
        }
        for (const auto& child : children_) {
            plot_aabb(child.first);
            child.second.plot_bvh(origin, svg, axis0, axis1);
        }
    }

    template <class TSize>
    void plot_svg(const std::string& filename, size_t axis0, size_t axis1) const {
        std::ofstream ofs{ filename };
        auto ab = aabb();
        Svg<TSize> svg{ ofs, ab.size()(axis0), ab.size()(axis1) };
        plot_bvh(ab.min(), svg, axis0, axis1);
        svg.finish();
        ofs.flush();
        if (ofs.fail()) {
            throw std::runtime_error("Could not write to file \"" + filename + '"');
        }
    }

    const std::list<std::pair<AxisAlignedBoundingBox<TData, tndim>, TPayload>>& data() const
    {
        return data_;
    }

    const std::list<std::pair<AxisAlignedBoundingBox<TData, tndim>, Bvh>>& children() const
    {
        return children_;
    }
private:
    FixedArray<TData, tndim> max_size_;
    size_t level_;
    std::list<std::pair<AxisAlignedBoundingBox<TData, tndim>, TPayload>> data_;
    std::list<std::pair<AxisAlignedBoundingBox<TData, tndim>, Bvh>> children_;
};

template <class TData, class TPayload, size_t tndim>
std::ostream& operator << (std::ostream& ostr, const Bvh<TData, TPayload, tndim>& bvh) {
    bvh.print(ostr, BvhPrintingOptions{});
    return ostr;
}

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
