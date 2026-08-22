#pragma once
#include <Mlib/Array/Non_Copying_Vector.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <concepts>
#include <optional>
#include <stdexcept>
#include <string>

namespace Mlib {

template <std::integral TIndex, class TValue>
class ContiguousIntegralMap {
    template <std::integral TIndex2, class TValue2>
    friend decltype(auto) cenumerate(ContiguousIntegralMap<TIndex2, TValue2>&& map);

    template <std::integral TIndex2, class TValue2>
    friend decltype(auto) cenumerate(ContiguousIntegralMap<TIndex2, TValue2>& map);

    template <std::integral TIndex2, class TValue2>
    friend decltype(auto) cenumerate(const ContiguousIntegralMap<TIndex2, TValue2>& map);
public:
    explicit ContiguousIntegralMap(std::string prefix)
        : prefix_{ std::move(prefix) }
    {}
    template <class... Args>
    void resize(TIndex n, Args&&... args) {
        elements_.resize(n, std::forward<Args>(args)...);
    }
    void reset() {
        for (auto& e : elements_) {
            e.reset();
        }
    }
    TIndex size() const {
        // The "resize" method's argument type ensures the validity of the cast.
        return (TIndex)elements_.size();
    }
    bool empty() const {
        return elements_.empty();
    }
    template <class... TArgs>
    TValue& add(TIndex i, TArgs&&... args) {
        if (i >= size()) {
            throw std::runtime_error(prefix_ + ": Index out of bounds during \"add\". Index: " +
                std::to_string(+i) + ", size: " + std::to_string(elements_.size()));
        }
        if (elements_.at(i).has_value()) {
            throw std::runtime_error(prefix_ + ": Index already set");
        }
        return elements_.at(i).emplace(std::forward<TArgs>(args)...);
    }
    TValue& get(TIndex i) {
        if (i >= elements_.size()) {
            throw std::runtime_error(prefix_ + ": Index out of bounds during \"get\". Index: " +
                std::to_string(+i) + ", size: " + std::to_string(elements_.size()));
        }
        auto& res = elements_[i];
        if (!res.has_value()) {
            throw std::runtime_error(prefix_ + ": Value not set");
        }
        return *res;
    }
    const TValue& get(TIndex i) const {
        return const_cast<ContiguousIntegralMap*>(this)->get(i);
    }
    std::optional<TValue>& get_optional(TIndex i) {
        if (i >= elements_.size()) {
            throw std::runtime_error(prefix_ + ": Index out of bounds during \"get_optional\". Index: " +
                std::to_string(+i) + ", size: " + std::to_string(elements_.size()));
        }
        return elements_[i];
    }
    decltype(auto) begin() {
        return elements_.begin();
    }
    decltype(auto) end() {
        return elements_.end();
    }
    decltype(auto) begin() const {
        return elements_.begin();
    }
    decltype(auto) end() const {
        return elements_.end();
    }
private:
    NonCopyingVector<std::optional<TValue>> elements_;
    std::string prefix_;
};

template <std::integral TIndex, class TValue>
decltype(auto) cenumerate(ContiguousIntegralMap<TIndex, TValue>&& map) {
    return tenumerate<TIndex>(std::move(map.elements_));
}

template <std::integral TIndex, class TValue>
decltype(auto) cenumerate(ContiguousIntegralMap<TIndex, TValue>& map) {
    return tenumerate<TIndex>(map.elements_);
}

template <std::integral TIndex, class TValue>
decltype(auto) cenumerate(const ContiguousIntegralMap<TIndex, TValue>& map) {
    return tenumerate<TIndex>(map.elements_);
}

}
