#pragma once
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Scene_Graph/Interfaces/IRenderable_Hider.hpp>

namespace Mlib {

class StaticRenderableHider final: public IRenderableHider {
public:
    explicit StaticRenderableHider(VariableAndHash<std::string> visible_name);
    virtual void process_input() override;
    virtual bool is_visible(const VariableAndHash<std::string>& name) override;

private:
    VariableAndHash<std::string> visible_name_;
};

}
