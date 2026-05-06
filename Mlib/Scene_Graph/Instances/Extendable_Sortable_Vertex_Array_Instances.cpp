#include "Extendable_Sortable_Vertex_Array_Instances.hpp"
#include <Mlib/Math/Sorted_Deferred.hpp>
#include <Mlib/Math/Transformation/Offset_And_YAngle.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Math/Transformation/Transformation_Variant.hpp>
#include <Mlib/Scene_Graph/Instances/Billboard_Container.hpp>
#include <Mlib/Scene_Graph/Instances/Sorted_Vertex_Array_Instances.hpp>
#include <stdexcept>

using namespace Mlib;

void ExtendableSortableVertexArrayInstances::insert(
    const InstanceLocation& i,
    float distance,
    BillboardId billboard_id)
{
    switch (i.data.index()) {
        case 0:
            if (billboard_id != BILLBOARD_ID_NONE) {
                throw std::runtime_error("Transformation matrix does not support billboard-ID");
            }
            transformed.emplace_back(distance, std::get<TransformationMatrix<SceneDir, ScenePos, 3>>(i.data).casted<float, float>());
            return;
        case 1:
            {
                const auto x = std::get<OffsetAndYAngle<SceneDir, ScenePos, 3>>(i.data).casted<float, float>();
                yangle.emplace_back(distance,
                    PositionAndYAngleAndBillboardId<float>{x.t, billboard_id, x.yangle});
            }
            return;
        case 2:
            lookat.emplace_back(distance,
                PositionAndBillboardId<float>{std::get<TranslationMatrix<ScenePos, 3>>(i.data).t.casted<float>(), billboard_id});
            return;
    }
    throw std::runtime_error("Unexpected instance location type");
}

SortedVertexArrayInstances ExtendableSortableVertexArrayInstances::sorted() const {
    SortedVertexArrayInstances result;
    const auto compare = [](const auto& a, const auto& b){ return a.distance < b.distance; };
    const auto proj = [](const auto& x){ return x.value; };
    result.transformed = sorted_deferred(transformed.begin(), transformed.end(), compare, proj);
    result.yangle = sorted_deferred(yangle.begin(), yangle.end(), compare, proj);
    result.lookat = sorted_deferred(lookat.begin(), lookat.end(), compare, proj);
    return result;
}
