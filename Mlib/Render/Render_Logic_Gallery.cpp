#include "Render_Logic_Gallery.hpp"
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

RenderLogicGallery::RenderLogicGallery() = default;

RenderLogicGallery::~RenderLogicGallery() = default;

void RenderLogicGallery::insert(const std::string& name, std::shared_ptr<FillWithTextureLogic> render_logic) {
    std::scoped_lock lock{mutex_};
    if (!render_logics_.try_emplace(name, std::move(render_logic)).second) {
        THROW_OR_ABORT("Render logic with name \"" + name + "\" already exists");
    }
}

std::shared_ptr<FillWithTextureLogic> RenderLogicGallery::operator [] (const std::string& name) const {
    std::shared_lock lock{mutex_};
    auto it = render_logics_.find(name);
    if (it == render_logics_.end()) {
        THROW_OR_ABORT("Could not find render logic with name \"" + name + '"');
    }
    return it->second;
}
