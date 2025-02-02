#pragma once
#include <Mlib/Scene_Graph/Interfaces/IRenderable_Hider.hpp>

namespace Mlib {

class StaticRenderableHider: public IRenderableHider {
public:
    explicit StaticRenderableHider(std::string visible_name);
    virtual void process_input() override;
    virtual bool is_visible(const std::string& name) override;

private:
    std::string visible_name_;
};

}
