#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IRelative_Movable.hpp>

namespace Mlib {

template <class T>
class DanglingRef;
class SceneNode;
class AdvanceTimes;

class RelativeTransformer: public DestructionObserver<DanglingRef<SceneNode>>, public IRelativeMovable, public IAdvanceTime, public DanglingBaseClass {
public:
    explicit RelativeTransformer(
        AdvanceTimes& advance_times,
        const FixedArray<float, 3>& v = {0, 0, 0},
        const FixedArray<float, 3>& w = {0, 0, 0});
    ~RelativeTransformer();
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) override;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, double, 3> get_new_relative_model_matrix() const override;
    virtual void advance_time(float dt, std::chrono::steady_clock::time_point time) override;
    virtual void notify_destroyed(DanglingRef<SceneNode> destroyed_object) override;
    AdvanceTimes& advance_times_;
    TransformationMatrix<float, double, 3> transformation_matrix_;
    FixedArray<float, 3> v_;
    FixedArray<float, 3> w_;
};

}
