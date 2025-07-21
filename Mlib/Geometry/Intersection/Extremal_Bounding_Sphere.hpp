#pragma once
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Intersection/Extremal_Bounding_Volume.hpp>
#include <Mlib/Os/Os.hpp>
#include <variant>

namespace Mlib {

template <class TData, size_t tndim>
class ExtremalBoundingSphere {
public:
    ExtremalBoundingSphere(ExtremalBoundingVolume other)
        : data_{ other }
    {}
    ExtremalBoundingSphere(const BoundingSphere<TData, tndim>& other)
        : data_{ other }
    {}
    ExtremalBoundingSphere& operator = (ExtremalBoundingVolume other) {
        data_ = other;
        return *this;
    }
    ExtremalBoundingSphere& operator = (const BoundingSphere<TData, tndim>& other) {
        data_ = other;
        return *this;
    }
    void extend(const ExtremalBoundingSphere<TData, tndim>& other) {
        if (auto* d = std::get_if<ExtremalBoundingVolume>(&data_)) {
            switch (*d) {
                case ExtremalBoundingVolume::EMPTY:
                    data_ = other.data_;
                    return;
                case ExtremalBoundingVolume::FULL:
                    return;
            }
            verbose_abort("Unknown bounding volume: " + std::to_string((int)(*d)));
        }
        if (const auto* d = std::get_if<ExtremalBoundingVolume>(&other.data_)) {
            switch (*d) {
                case ExtremalBoundingVolume::EMPTY:
                    return;
                case ExtremalBoundingVolume::FULL:
                    data_ = ExtremalBoundingVolume::FULL;
                    return;
            }
            verbose_abort("Unknown bounding volume: " + std::to_string((int)(*d)));
        }
        if (auto* lhs = std::get_if<BoundingSphere<TData, tndim>>(&data_)) {
            if (auto* rhs = std::get_if<BoundingSphere<TData, tndim>>(&other.data_)) {
                lhs->extend(*rhs);
                return;
            }
        }
        verbose_abort("Unknown bounding volume");
    }
    template <class TResult>
    ExtremalBoundingSphere<TResult, tndim> casted() const {
        if (const auto* d = std::get_if<ExtremalBoundingVolume>(&data_)) {
            return *d;
        }
        if (const auto* d = std::get_if<BoundingSphere<TData, tndim>>(&data_)) {
            return d->template casted<TResult>();
        }
        verbose_abort("Unknown bounding volume");
    }
    bool empty() const {
        if (const auto* d = std::get_if<ExtremalBoundingVolume>(&data_)) {
            return *d == ExtremalBoundingVolume::EMPTY;
        }
        return false;
    }
    bool full() const {
        if (const auto* d = std::get_if<ExtremalBoundingVolume>(&data_)) {
            return *d == ExtremalBoundingVolume::FULL;
        }
        return false;
    }
    const BoundingSphere<TData, tndim>& data() const {
        if (const auto* d = std::get_if<BoundingSphere<TData, tndim>>(&data_)) {
            return *d;
        }
        verbose_abort("BoundingSphere is either empty or full");
    }
private:
    std::variant<ExtremalBoundingVolume, BoundingSphere<TData, tndim>> data_;
};

}
