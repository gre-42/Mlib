#pragma once
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>

namespace Mlib {

class LayoutConstraint;

class LayoutConstraints {
public:
    LayoutConstraints();
    ~LayoutConstraints();
    LayoutConstraint& get(const std::string& name) const;
    void insert(const std::string& name, std::unique_ptr<LayoutConstraint>&& constraint);
private:
    std::map<std::string, std::unique_ptr<LayoutConstraint>> constraints_;
    mutable std::shared_mutex mutex_;
};

}
