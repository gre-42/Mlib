#pragma once
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>

namespace Mlib {

class ILayoutPixels;

class LayoutConstraints {
public:
    LayoutConstraints();
    ~LayoutConstraints();
    ILayoutPixels& get_pixels(const std::string& name) const;
    void set_pixels(const std::string& name, std::unique_ptr<ILayoutPixels>&& position);
private:
    mutable std::shared_mutex mutex_;
    std::map<std::string, std::unique_ptr<ILayoutPixels>> pixels_;
};

}
