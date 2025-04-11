#pragma once
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations_Fwd.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <memory>

namespace Mlib {

class ReadPixelsLogicKeys;
class ButtonStates;

enum class ReadPixelsRole {
    NONE = 0,
    INTERMEDIATE = 1 << 0,
    SCREENSHOT = 1 << 1
};

inline bool any(ReadPixelsRole role) {
    return role != ReadPixelsRole::NONE;
}

inline ReadPixelsRole operator & (ReadPixelsRole a, ReadPixelsRole b) {
    return (ReadPixelsRole)((int)a & (int)b);
}

inline ReadPixelsRole operator | (ReadPixelsRole a, ReadPixelsRole b) {
    return (ReadPixelsRole)((int)a | (int)b);
}

class ReadPixelsLogic: public RenderLogic {
public:
    explicit ReadPixelsLogic(
        RenderLogic& child_logic,
        const ButtonStates& button_states,
        const LockableKeyConfigurations& key_configurations,
        ReadPixelsRole role);
    ~ReadPixelsLogic();

    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual bool render_optional_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id,
        const RenderSetup* setup) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    RenderLogic& child_logic_;
    std::unique_ptr<ReadPixelsLogicKeys> keys_;
    ReadPixelsRole role_;
};

}
