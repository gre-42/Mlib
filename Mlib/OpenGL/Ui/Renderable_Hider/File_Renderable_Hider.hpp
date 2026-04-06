#pragma once
#include <Mlib/OpenGL/Key_Bindings/Lockable_Key_Configurations.hpp>
#include <Mlib/OpenGL/Ui/Button_Press.hpp>
#include <Mlib/OpenGL/Ui/Renderable_Hider/Unordered_Set_Of_Strings.hpp>
#include <Mlib/Scene_Graph/Interfaces/IRenderable_Hider.hpp>
#include <Mlib/Threads/Safe_Atomic_Shared_Mutex.hpp>
#include <filesystem>

namespace Mlib {

class FileRenderableHider final: public IRenderableHider {
public:
    explicit FileRenderableHider(
        const ButtonStates& button_states,
        std::filesystem::path filename);
    virtual void process_input() override;
    virtual bool is_visible(const VariableAndHash<std::string>& name) override;

private:
    std::filesystem::path filename_;
    UnorderedSetOfStrings available_names_;
    UnorderedSetOfStrings visible_names_;
    LockableKeyConfigurations key_configurations_;
    ButtonPress load_;
    ButtonPress save_;
    SafeAtomicSharedMutex mutex_;
};

}
