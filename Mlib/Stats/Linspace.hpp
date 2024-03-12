#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
std::pair<TData, TData> linspace_multipliers(size_t i, size_t count) {
    return std::make_pair(
        TData(count - i - 1) / (TData)(count - 1),
        TData(i) / (TData)(count - 1));
}

template <class TData>
class LinspaceIterator;

template <class TData>
class Linspace {
public:
    Linspace(const TData& from, const TData& to, size_t count)
    : from_{from}, to_{to}, count_{count}
    {}
    TData operator [] (size_t i) const {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        return (from_ * TData(count_ - i - 1) + to_ * TData(i)) / TData(count_ - 1);
#pragma GCC diagnostic pop
    }
    size_t length() const {
        return count_;
    }
    LinspaceIterator<TData> begin() const {
        return LinspaceIterator<TData>{*this, 0};
    }
    LinspaceIterator<TData> end() const {
        return LinspaceIterator<TData>{*this, count_};
    }
private:
    TData from_;
    TData to_;
    size_t count_;
};

template <class TData>
class LinspaceIterator {
public:
    LinspaceIterator(const Linspace<TData>& linspace, size_t i)
    : linspace_{linspace},
      i_{i}
    {}
    TData operator * () const {
        return linspace_[i_];
    }
    LinspaceIterator& operator ++ () {
        ++i_;
        return *this;
    }
    bool operator != (const LinspaceIterator& other) const {
        return i_ != other.i_;
    }
private:
    const Linspace<TData>& linspace_;
    size_t i_;
};

template <class TFloat>
Array<std::pair<TFloat, TFloat>> linspace_multipliers(size_t count) {
    Array<std::pair<TFloat, TFloat>> result{ArrayShape{count}};
    if (count == 1) {
        result = std::make_pair(1 / TFloat(2), 1 / TFloat(2));
    } else {
        for (size_t i = 0; i < count; ++i) {
            result(i) = linspace_multipliers<TFloat>(i, count);
        }
    }
    return result;
}

template <class TData>
Array<TData> linspace(const TData& from, const TData& to, size_t count) {
    Array<TData> result{ArrayShape{count}};
    if (count == 1) {
        result = (from + to) / 2;
    } else {
        for (size_t i = 0; i < count; ++i) {
            result(i) = (from * TData(count - i - 1) + to * TData(i)) / TData(count - 1);
        }
    }
    return result;
}

}
