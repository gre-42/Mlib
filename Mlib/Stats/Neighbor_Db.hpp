#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Stats/Sort.hpp>
#include <Mlib/Stats/Variance.hpp>
#include <list>
#include <set>

namespace Mlib {

template <class TData>
class NeighborDb1d {
public:
    NeighborDb1d(const Array<TData>& values)
    : ids_(argsort(values)),
      sorted_values(values[ids_]) {}

    Array<size_t> get_neighbors(const TData& value, const TData& radius) const {
        if (ids_.length() == 0) {
            return Array<size_t>{ArrayShape{0}};
        }
        const TData* low_it = std::lower_bound(
            &sorted_values(0),
            &sorted_values(0) + sorted_values.length(),
            value - radius);
        const TData* upp_it = std::upper_bound(
            &sorted_values(0),
            &sorted_values(0) + sorted_values.length(),
            value + radius);
        size_t low_id = size_t(low_it - &sorted_values(0));
        size_t upp_id = size_t(upp_it - &sorted_values(0));
        if (low_id == ids_.length()) {
            return Array<size_t>{ArrayShape{0}};
        } else {
            return ids_.row_range(low_id, upp_id);
        }
    }
private:
    Array<size_t> ids_;
    Array<TData> sorted_values;
};

template <class TData>
class NeighborDb {
public:
    NeighborDb(const std::list<Array<TData>>& dbd, bool standardize = false)
    {
        {
            Array<TData> adb{dbd};
            assert(adb.ndim() == 2);
            std_.resize(ArrayShape{adb.shape(1)});
            size_t i = 0;
            for (const Array<TData>& col : adb.T()) {
                std_(i) = standardize ? stddev(col) : 1;
                nb_.push_back(NeighborDb1d<TData>{col});
                ++i;
            }
        }
        {
            vdb_.reserve(dbd.size());
            for (const Array<TData>& e : dbd) {
                vdb_.push_back(e);
            }
        }
    }

    size_t count(const Array<TData>& x, const TData& radius) {
        assert(x.length() == nb_.size());
        std::set<size_t> old_ids;
        std::set<size_t> new_ids;
        size_t i = 0;
        for (const NeighborDb1d<TData>& nb1d : nb_) {
            Array<size_t> ids = nb1d.get_neighbors(x(i), radius * std_(i));
            if (i == 0) {
                for (size_t id : ids.element_iterable()) {
                    new_ids.insert(id);
                }
            } else {
                old_ids.swap(new_ids);
                new_ids.clear();
                for (size_t id : ids.element_iterable()) {
                    if (old_ids.find(id) != old_ids.end()) {
                        new_ids.insert(id);
                    }
                }
            }
            ++i;
        }
        size_t result = 0;
        float radius_squared = squared(radius);
        for (size_t id : new_ids) {
            TData dist = 0;
            for (size_t i = 0; i < x.length(); ++i) {
                dist += squared((vdb_[id](i) - x(i)) / std_(i));
            }
            if (dist <= radius_squared) {
                ++result;
            }
            //if (sum(squared(vdb_[id] - x)) <= radius * radius) {
            //    ++result;
            //}
        }
        return result;
    }

    size_t count_slow(const Array<TData>& x, const TData& radius) {
        size_t result = 0;
        for (const Array<TData>& p : vdb_) {
            if (sum(squared(p - x)) <= radius * radius) {
                ++result;
            }
        }
        return result;
    }

private:
    std::vector<Array<TData>> vdb_;
    std::list<NeighborDb1d<TData>> nb_;
    Array<TData> std_;
};

}
