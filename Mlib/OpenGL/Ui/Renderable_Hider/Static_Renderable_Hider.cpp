
#include "Static_Renderable_Hider.hpp"
#include <Mlib/OpenGL/Batch_Renderers/Special_Renderable_Names.hpp>

using namespace Mlib;


StaticRenderableHider::StaticRenderableHider(VariableAndHash<std::string> visible_name)
    : visible_name_{ std::move(visible_name) }
{}

void StaticRenderableHider::process_input() {}

bool StaticRenderableHider::is_visible(const VariableAndHash<std::string>& name) {
    if (name->empty() || (name == AAR_NAME)) {
        return true;
    }
    if (visible_name_->empty()) {
        return true;
    }
    return name == visible_name_;
}
