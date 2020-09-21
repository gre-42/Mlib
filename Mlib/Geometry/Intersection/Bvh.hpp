#pragma once
#include <Mlib/Geometry/Intersection/Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <list>
#include <map>
#include <regex>
#include <vector>

namespace Mlib {

template <class TData, class TPayload, size_t tndim>
class Bvh {
public:
    explicit Bvh(const FixedArray<TData, tndim>& max_size, size_t max_children)
    : level_{0},
      max_size_{max_size},
      max_children_{max_children}
    {
        if (max_children_ < 2) {
            // building [old, new] on overflow requires 2 children.
            throw std::runtime_error("max_children must be >= 2");
        }
    }
    void insert(
        const BoundingBox<TData, tndim>& bounding_box,
        const std::string& category,
        const TPayload& data)
    {
        if (data_[category].size() < max_children_) {
            data_.at(category).push_back({bounding_box, data});
            return;
        }
        for(auto& c : children_) {
            BoundingBox<TData, tndim> bb = c.first;
            bb.extend(bounding_box);
            if (all(bb.size() > TData(level_) * max_size_)) {
                c.first = bb;
                c.second.insert(bounding_box, category, data);
                return;
            }
        }
        if (children_.size() == max_children_) {
            auto bbox = this->bounding_box();
            children_.push_back({bbox, std::move(*this)});
            ++level_;
        }
        Bvh bvh{max_size_, max_children_};
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
    void print(std::ostream& ostr, size_t rec = 0) const {
        std::string indent(rec, ' ');
        ostr << indent << "level " << level_ << std::endl;
        ostr << indent << "data " << data_.size() << std::endl;
        for(const auto& d : data_) {
            ostr << indent << d.first << " " << std::endl;
            for(const auto& v : d.second) {
                v.first.print(ostr, rec + 1);
            }
        }
        ostr << indent << "children " << children_.size() << std::endl;
        for(const auto& c : children_) {
            c.first.print(ostr, rec + 1);
            c.second.print(ostr, rec + 1);
        }
    }
private:
    size_t level_;
    FixedArray<TData, tndim> max_size_;
    size_t max_children_;
    std::map<std::string, std::list<std::pair<BoundingBox<TData, tndim>, TPayload>>> data_;
    std::list<std::pair<BoundingBox<TData, tndim>, Bvh>> children_;
};

template <class TData, class TPayload, size_t tndim>
std::ostream& operator << (std::ostream& ostr, const Bvh<TData, TPayload, tndim>& bvh) {
    bvh.print(ostr);
    return ostr;
}

}
