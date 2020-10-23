#pragma once
#include <Mlib/Geometry/Intersection/Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <list>
#include <map>
#include <ostream>
#include <regex>
#include <vector>

namespace Mlib {

struct BvhPrintingOptions {
    bool level = true;
    bool bounding_box = true;
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
        const BoundingBox<TData, tndim>& bounding_box,
        const std::string& category,
        const TPayload& data)
    {
        if (level_ == 0) {
            data_[category].push_back({bounding_box, data});
            return;
        }
        for(auto& c : children_) {
            BoundingBox<TData, tndim> bb = c.first;
            bb.extend(bounding_box);
            if (all(bb.size() <= TData(level_) * max_size_)) {
                c.first = bb;
                c.second.insert(bounding_box, category, data);
                return;
            }
        }
        Bvh bvh{max_size_, level_ - 1};
        bvh.insert(bounding_box, category, data);
        children_.push_back({bounding_box, bvh});
    }
    template <class TVisitor>
    void visit(const BoundingSphere<TData, tndim>& sphere, const TVisitor& visitor, const std::regex& filter = std::regex{""}) const {
        for(const auto& d : data_) {
            if (std::regex_search(d.first, filter)) {
                for(const auto& v : d.second) {
                    if (v.first.intersects(sphere)) {
                        visitor(d.first, v.second);
                    }
                }
            }
        }
        for(const auto& c : children_) {
            if (c.first.intersects(sphere)) {
                c.second.visit(sphere, visitor);
            }
        }
    }
    BoundingBox<TData, tndim> bounding_box() const {
        BoundingBox<TData, tndim> result;
        for(const auto& d : data_) {
            for(const auto& v : d.second) {
                result.extend(v.first);
            }
        }
        for(const auto& c : children_) {
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
            for(const auto& d : data_) {
                ostr << indent << d.first << " " << d.second.size() << std::endl;
                for(const auto& v : d.second) {
                    if (opts.bounding_box) {
                        v.first.print(ostr, rec + 1);
                    }
                }
            }
        }
        if (opts.children) {
            ostr << indent << "children " << children_.size() << std::endl;
            for(const auto& c : children_) {
                if (opts.bounding_box) {
                    c.first.print(ostr, rec + 1);
                }
                c.second.print(ostr, opts, rec + 1);
            }
        }
    }
    float search_time() const {
        float res = children_.size();
        for(const auto& c : children_) {
            res += c.second.search_time() / children_.size();
        }
        for(const auto& d : data_) {
            res += d.second.size();
        }
        return res;
    }
private:
    FixedArray<TData, tndim> max_size_;
    size_t level_;
    std::map<std::string, std::list<std::pair<BoundingBox<TData, tndim>, TPayload>>> data_;
    std::list<std::pair<BoundingBox<TData, tndim>, Bvh>> children_;
};

template <class TData, class TPayload, size_t tndim>
std::ostream& operator << (std::ostream& ostr, const Bvh<TData, TPayload, tndim>& bvh) {
    bvh.print(ostr, BvhPrintingOptions{});
    return ostr;
}

}
