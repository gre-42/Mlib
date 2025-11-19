#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Scene/Json_User_Function.hpp>
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/INode_Hider.hpp>

namespace Mlib {

class PhysicsNodeHiderWithEvent: public INodeHider, public DestructionObserver<SceneNode&>, public IAdvanceTime, public virtual DanglingBaseClass {
public:
    PhysicsNodeHiderWithEvent(
        DanglingBaseClassRef<SceneNode> node_to_hide,
        DanglingBaseClassRef<SceneNode> camera_node);

    virtual ~PhysicsNodeHiderWithEvent() override;
    virtual void notify_destroyed(SceneNode& destroyed_object) override;
    virtual bool node_shall_be_hidden(
        const DanglingBaseClassPtr<const SceneNode>& camera_node,
        const ExternalRenderPass& external_render_pass) const override;
    virtual void advance_time(float dt, const StaticWorld& world) override;

private:
    DanglingBaseClassPtr<SceneNode> node_to_hide_;
    DanglingBaseClassPtr<SceneNode> camera_node_;
};

class InsertPhysicsNodeHider: public LoadPhysicsSceneInstanceFunction {
public:
    explicit InsertPhysicsNodeHider(PhysicsScene& physics_scene);
    void execute(const LoadSceneJsonUserFunctionArgs& args);
};

}
