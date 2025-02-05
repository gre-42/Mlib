#pragma once
#include <Mlib/Threads/Safe_Atomic_Shared_Mutex.hpp>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

class ILayoutPixels;

class LayoutConstraints {
public:
    LayoutConstraints();
    ~LayoutConstraints();
    ILayoutPixels& get_pixels(const std::string& name) const;
    void set_pixels(std::string name, std::unique_ptr<ILayoutPixels>&& position);
private:
    mutable SafeAtomicSharedMutex mutex_;
    std::map<std::string, std::unique_ptr<ILayoutPixels>> pixels_;
};

}
