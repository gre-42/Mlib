#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <list>
#include <ostream>
#include <vector>
#include <iomanip>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

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

    void insert(
        const AxisAlignedBoundingBox<TData, tndim>& aabb,
        const TPayload& data)
    {
        if (level_ == 0) {
            data_.push_back({aabb, data});
            return;
        }
        for (auto& c : children_) {
            AxisAlignedBoundingBox<TData, tndim> bb = c.first;
            bb.extend(aabb);
            if (all(bb.size() <= TData(level_) * max_size_)) {
            // if (all(bb.size() <= max_size_ * std::pow(TData(2), TData(level_)))) {
                c.first = bb;
                c.second.insert(aabb, data);
                return;
            }
        }
        Bvh bvh{max_size_, level_ - 1};
        bvh.insert(aabb, data);
        children_.push_back({aabb, bvh});
    }

    template <class TVisitor>
    void visit(const BoundingSphere<TData, tndim>& sphere, const TVisitor& visitor) const {
        for (const auto& d : data_) {
            if (d.first.intersects(sphere)) {
                visitor(d.second);
            }
        }
        for (const auto& c : children_) {
            if (c.first.intersects(sphere)) {
                c.second.visit(sphere, visitor);
            }
        }
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
    void visit_all(const TVisitor& visitor) const {
        for (const auto& x : data_) {
            visitor(x);
        }
        for (const auto& c : children_) {
            c.second.visit_all(visitor);
        }
    }

    Bvh repackaged(const FixedArray<TData, tndim>& max_size, size_t level) const
    {
        Bvh<TData, TPayload, tndim> result{ max_size, level };
        visit_all([&](const auto& d) { result.insert(d.first, d.second); });
        return result;
    }

    void optimize_search_time(std::ostream& ostr) const {
        for (TData max_size_fac = (TData)0.1; max_size_fac < 10; max_size_fac *= 2) {
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
