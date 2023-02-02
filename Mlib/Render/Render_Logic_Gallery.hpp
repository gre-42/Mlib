#pragma once
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>

namespace Mlib {

class FillWithTextureLogic;

class RenderLogicGallery {
public:
    RenderLogicGallery();
    ~RenderLogicGallery();
    void insert(const std::string& name, std::shared_ptr<FillWithTextureLogic> render_logic);
    std::shared_ptr<FillWithTextureLogic> operator [] (const std::string& name) const;
private:
    mutable std::shared_mutex mutex_;
    std::map<std::string, std::shared_ptr<FillWithTextureLogic>> render_logics_;
};

}
