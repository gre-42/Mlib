#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Box.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Min_Max.hpp>

namespace Mlib {

template <class TData, size_t tndim, class TUserData>
class Octree {
public:
    void insert(const BoundingBox<TData, tndim>& bbox, const TUserData& user_data) {
        elements_.push_back(std::make_pair(bbox, user_data));
        bbox_.extend(bbox);
        if (elements_.size() > 10) {
            while(!elements_.empty()) {
                auto& e = elements_.front();
                for (size_t i = 0; i < tndim; ++i) {

                }
                auto idn = (e.first.bbox_.min_ > cutoff_);
                size_t id = 0;
                size_t s;
                for (size_t i = 0; i < tndim; ++i) {
                    id = s * id + id;
                    s *= 2;
                }
                children_(id)->insert(e.first, e.second);
            }
        }
    }
    BoundingBox<TData, tndim> bbox_;
private:
    FixedArray<std::unique_ptr<Octree>, 2, tndim> children_;
    FixedArray<TData, tndim> cutoff_;
    std::list<std::pair<BoundingBox<TData, tndim>, TUserData>> elements_;
};

}
