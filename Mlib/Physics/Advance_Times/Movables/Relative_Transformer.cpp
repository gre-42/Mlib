#include "Relative_Transformer.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

RelativeTransformer::RelativeTransformer(
    const FixedArray<float, 3>& v,
    const FixedArray<float, 3>& w)
    : transformation_matrix_{
          fixed_nans<float, 3, 3>(),
          fixed_nans<ScenePos, 3>()
    }
    , v_{ v }
    , w_{ w }
{}

RelativeTransformer::~RelativeTransformer() {
    on_destroy.clear();
}

void RelativeTransformer::set_initial_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix)
{
    transformation_matrix_ = relative_model_matrix;
}

void RelativeTransformer::set_updated_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix)
{
    transformation_matrix_.t = relative_model_matrix.t;
}

void RelativeTransformer::set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix)
{
    // do nothing
}

TransformationMatrix<float, ScenePos, 3> RelativeTransformer::get_new_relative_model_matrix() const
{
    return transformation_matrix_;
}

void RelativeTransformer::advance_time(float dt, const StaticWorld& world) {
    transformation_matrix_.t += (dt * v_).casted<ScenePos>();
    transformation_matrix_.R = dot2d(rodrigues1(dt * w_), transformation_matrix_.R);
}

void RelativeTransformer::notify_destroyed(SceneNode& destroyed_object) {
    if (destroyed_object.has_relative_movable()) {
        if (&destroyed_object.get_relative_movable() != this) {
            verbose_abort("Unexpected relative movable");
        }
        destroyed_object.clear_relative_movable();
    }
    global_object_pool.remove(this);
}
