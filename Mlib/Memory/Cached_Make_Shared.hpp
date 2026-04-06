#pragma once
#include <memory>
#include <utility>

namespace Mlib {

template <class T>
class CachedMakeShared {
public:
    template <class... TArgs>
    std::shared_ptr<T> operator () (TArgs&&... args) {
        if (ptr_ == nullptr) {
            ptr_ = std::make_shared<T>(std::forward<TArgs>(args)...);
        }
        return ptr_;
    }
private:
    std::shared_ptr<T> ptr_;
};

}
