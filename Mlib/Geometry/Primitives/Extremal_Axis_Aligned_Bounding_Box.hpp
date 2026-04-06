#pragma once
#include <Mlib/Geometry/Primitives/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Primitives/Extremal_Bounding_Volume.hpp>
#include <Mlib/Os/Os.hpp>
#include <ostream>
#include <variant>

namespace Mlib {

template <class TData, size_t tndim>
class ExtremalAxisAlignedBoundingBox {
public:
    ExtremalAxisAlignedBoundingBox(ExtremalBoundingVolume other)
        : data_{ other }
    {}
    ExtremalAxisAlignedBoundingBox(const AxisAlignedBoundingBox<TData, tndim>& other)
        : data_{ other }
    {}
    ExtremalAxisAlignedBoundingBox& operator = (ExtremalBoundingVolume other) {
        data_ = other;
        return *this;
    }
    ExtremalAxisAlignedBoundingBox& operator = (const AxisAlignedBoundingBox<TData, tndim>& other) {
        data_ = other;
        return *this;
    }
    void extend(const ExtremalAxisAlignedBoundingBox<TData, tndim>& other) {
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
        if (auto* lhs = std::get_if<AxisAlignedBoundingBox<TData, tndim>>(&data_)) {
            if (auto* rhs = std::get_if<AxisAlignedBoundingBox<TData, tndim>>(&other.data_)) {
                lhs->extend(*rhs);
                return;
            }
        }
        verbose_abort("Unknown bounding volume");
    }
    template <class TResult>
    ExtremalAxisAlignedBoundingBox<TResult, tndim> casted() const {
        if (const auto* d = std::get_if<ExtremalBoundingVolume>(&data_)) {
            return *d;
        }
        if (const auto* d = std::get_if<AxisAlignedBoundingBox<TData, tndim>>(&data_)) {
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
    const AxisAlignedBoundingBox<TData, tndim>& data() const {
        if (const auto* d = std::get_if<AxisAlignedBoundingBox<TData, tndim>>(&data_)) {
            return *d;
        }
        verbose_abort("ExtremalAxisAlignedBoundingBox is empty or full");
    }
    void print(std::ostream& ostr) const {
        if (auto* d = std::get_if<ExtremalBoundingVolume>(&data_)) {
            switch (*d) {
            case ExtremalBoundingVolume::EMPTY:
                ostr << "empty";
                return;
            case ExtremalBoundingVolume::FULL:
                ostr << "full";
                return;
            }
            verbose_abort("Unknown bounding volume: " + std::to_string((int)(*d)));
        }
        if (auto* d = std::get_if<AxisAlignedBoundingBox<TData, tndim>>(&data_)) {
            ostr << *d;
            return;
        }
        verbose_abort("Unknown bounding volume");
    }
private:
    std::variant<ExtremalBoundingVolume, AxisAlignedBoundingBox<TData, tndim>> data_;
};

template <class TData, size_t tndim>
std::ostream& operator << (std::ostream& ostr, const ExtremalAxisAlignedBoundingBox<TData, tndim>& aabb) {
    aabb.print(ostr);
    return ostr;
}

}
