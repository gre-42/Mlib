#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <list>
#include <ostream>
#include <vector>

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
