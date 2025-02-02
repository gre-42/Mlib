#pragma once
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <map>
#include <memory>
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
    mutable SafeAtomicRecursiveSharedMutex mutex_;
    std::map<std::string, std::shared_ptr<FillWithTextureLogic>> render_logics_;
};

}
