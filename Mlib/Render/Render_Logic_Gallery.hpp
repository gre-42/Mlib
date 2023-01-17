#pragma once
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>

namespace Mlib {

class RenderLogic;

class RenderLogicGallery {
public:
    RenderLogicGallery();
    ~RenderLogicGallery();
    void insert(const std::string& name, std::unique_ptr<RenderLogic>&& render_logic);
    RenderLogic& operator [] (const std::string& name) const;
private:
    mutable std::shared_mutex mutex_;
    std::map<std::string, std::unique_ptr<RenderLogic>> render_logics_;
};

}
