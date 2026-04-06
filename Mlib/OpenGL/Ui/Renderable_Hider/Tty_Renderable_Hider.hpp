#pragma once
#include <Mlib/OpenGL/Key_Bindings/Lockable_Key_Configurations.hpp>
#include <Mlib/OpenGL/Ui/Button_Press.hpp>
#include <Mlib/OpenGL/Ui/Renderable_Hider/Set_Of_Strings.hpp>
#include <Mlib/Scene_Graph/Interfaces/IRenderable_Hider.hpp>
#include <Mlib/Threads/Safe_Atomic_Shared_Mutex.hpp>

namespace Mlib {

class TtyRenderableHider final: public IRenderableHider {
public:
    explicit TtyRenderableHider(const ButtonStates& button_states);
    virtual void process_input() override;
    virtual bool is_visible(const VariableAndHash<std::string>& name) override;

private:
    std::string first_visible_name_;
    SetOfStrings available_names_;
    LockableKeyConfigurations key_configurations_;
    ButtonPress decrease_;
    ButtonPress increase_;
    ButtonPress decrease_much_;
    ButtonPress increase_much_;
    ButtonPress show_only_1_;
    SafeAtomicSharedMutex mutex_;
};

}
