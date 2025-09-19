#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Precision.hpp>

namespace Mlib {

struct RenderSetup {
    FixedArray<ScenePos, 4, 4> vp;
    TransformationMatrix<float, ScenePos, 3> iv;
    std::unique_ptr<Camera> camera;
    DanglingBaseClassPtr<const SceneNode> camera_node;
};

}
