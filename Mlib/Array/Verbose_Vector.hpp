#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <cstddef>
#include <string>
#include <vector>

namespace Mlib {

template <class T>
class VerboseVector {
public:
    VerboseVector(std::string name)
        : name_{ std::move(name) }
    {}
    void reserve(size_t n) {
        data_.reserve(n);
    }
    template <class... Args>
    decltype(auto) emplace_back(Args&&... args) {
        return data_.emplace_back(std::forward<Args>(args)...);
    }
    decltype(auto) get(size_t i) {
        if (i >= data_.size()) {
            THROW_OR_ABORT(
                name_ + ": index (" + std::to_string(i) +
                ") is out of bounds (" + std::to_string(data_.size()) + ')');
        }
        return data_[i];
    }
    decltype(auto) get(size_t i) const {
        return const_cast<VerboseVector<T>&>(*this).get(i);
    }
    size_t size() const {
        return data_.size();
    }
private:
    std::string name_;
    std::vector<T> data_;
};
    
}
