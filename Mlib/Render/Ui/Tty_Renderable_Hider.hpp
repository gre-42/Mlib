#pragma once
#include <Mlib/Render/Key_Bindings/Key_Configurations.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Scene_Graph/Interfaces/IRenderable_Hider.hpp>
#include <Mlib/Threads/Safe_Atomic_Shared_Mutex.hpp>
#include <set>

namespace Mlib {

class TtyRenderableHider: public IRenderableHider {
public:
    explicit TtyRenderableHider(const ButtonStates& button_states);
    virtual void process_input() override;
    virtual bool is_visible(const std::string& name) override;

private:
    std::string first_visible_name_;
    std::set<std::string> available_names_;
    KeyConfigurations key_configurations_;
    ButtonPress decrease_;
    ButtonPress increase_;
    ButtonPress decrease_much_;
    ButtonPress increase_much_;
    ButtonPress show_only_1_;
    SafeAtomicSharedMutex mutex_;
};

}
