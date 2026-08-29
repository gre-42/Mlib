#pragma once
#include <Mlib/Geometry/Billboard_Id.hpp>
#include <Mlib/OpenGL/Instance_Handles/Buffer_Background_Copy.hpp>
#include <Mlib/Scene_Graph/Instances/Sorted_Vertex_Array_Instances.hpp>
#include <vector>

namespace Mlib {

enum class TransformationMode;

class StaticBillboardIds {
    StaticBillboardIds(const StaticBillboardIds &) = delete;
    StaticBillboardIds &operator=(const StaticBillboardIds &) = delete;

public:
    StaticBillboardIds(
        TransformationMode transformation_mode,
        const SortedVertexArrayInstances& instances,
        BillboardId num_billboard_atlas_components,
        size_t capacity);
    ~StaticBillboardIds();
    void update(const SortedVertexArrayInstances& instances);
    bool copy_in_progress() const;
    void wait() const;
    void bind(GLuint attribute_index) const;
    BackgroundCopyState state() const;
    size_t size() const;

private:
    void wait_and_assign(const SortedVertexArrayInstances& instances);
    TransformationMode transformation_mode_;
    BillboardId num_billboard_atlas_components_;
    size_t capacity_;
    std::vector<BillboardId> billboard_ids_;
    mutable BufferForegroundCopy buffer_;
};

}
