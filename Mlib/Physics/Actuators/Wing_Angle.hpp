#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IRelative_Movable.hpp>

namespace Mlib {

class WingAngle: public DestructionObserver<DanglingRef<SceneNode>>, public IRelativeMovable, public DanglingBaseClass {
public:
    explicit WingAngle(DanglingPtr<SceneNode> node, float& angle, const FixedArray<float, 3>& rotation_axis);
    virtual ~WingAngle() override;

    // DestructionObserver
    virtual void notify_destroyed(DanglingRef<SceneNode> destroyed_object) override;

    // IRelativeMovable
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) override;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, double, 3> get_new_relative_model_matrix() const override;
private:
    DanglingPtr<SceneNode> node_;
    float& angle_;
    FixedArray<float, 3> rotation_axis_;
    FixedArray<double, 3> position_;
};

}
