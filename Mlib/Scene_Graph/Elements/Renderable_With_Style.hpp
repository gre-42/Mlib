#pragma once
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Threads/Safe_Atomic_Shared_Mutex.hpp>
#include <memory>

namespace Mlib {

class Renderable;

class RenderableWithStyle {
public:
    explicit RenderableWithStyle(std::shared_ptr<const Renderable> renderable);
    inline const Renderable* operator -> () const {
        return renderable_.get();
    }
    inline operator const Renderable* () const {
        return renderable_.get();
    }
    const ColorStyle* style(
        const std::list<const ColorStyle*>& color_styles,
        const std::string& name) const;
private:
    std::shared_ptr<const Renderable> renderable_;
    mutable SafeAtomicSharedMutex style_hash_mutex_;
    mutable ColorStyle style_;
    mutable size_t style_hash_;
};

}