
#include "Extendable_Vertex_Array_Instances.hpp"
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Math/Transformation/Transformation_Variant.hpp>
#include <Mlib/Scene_Graph/Instances/Billboard_Container.hpp>
#include <Mlib/Scene_Graph/Instances/Sorted_Vertex_Array_Instances.hpp>
#include <stdexcept>

using namespace Mlib;

void ExtendableVertexArrayInstances::insert(
    const InstanceLocation& i,
    BillboardId billboard_id)
{
    switch (i.data.index()) {
        case 0:
            if (billboard_id != BILLBOARD_ID_NONE) {
                throw std::runtime_error("Transformation matrix does not support billboard-ID");
            }
            transformed.emplace_back(std::get<TransformationMatrix<SceneDir, ScenePos, 3>>(i.data).casted<float, float>());
            return;
        case 1:
            {
                const auto x = std::get<OffsetAndYAngle<SceneDir, ScenePos, 3>>(i.data).casted<float, float>();
                yangle.emplace_back(x.t, billboard_id, x.yangle);
            }
            return;
        case 2:
            lookat.emplace_back(std::get<TranslationMatrix<ScenePos, 3>>(i.data).t.casted<float>(), billboard_id);
            return;
    }
    throw std::runtime_error("Unexpected instance location type");
}

SortedVertexArrayInstances ExtendableVertexArrayInstances::vectorized() const {
    return SortedVertexArrayInstances{
        .transformed = transformed.to_vector(),
        .yangle = yangle.to_vector(),
        .lookat = lookat.to_vector()
    };
}
