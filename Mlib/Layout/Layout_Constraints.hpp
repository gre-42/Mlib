#pragma once
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>

namespace Mlib {

class ILayoutScalar;

class LayoutConstraints {
public:
    LayoutConstraints();
    ~LayoutConstraints();
    ILayoutScalar& get_scalar(const std::string& name) const;
    void set_scalar(const std::string& name, std::unique_ptr<ILayoutScalar>&& constraint);
private:
    mutable std::shared_mutex mutex_;
    std::map<std::string, std::unique_ptr<ILayoutScalar>> constraints_;
};

}
