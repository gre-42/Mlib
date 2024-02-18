#pragma once
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Billboard_Sequence.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Billboard_Ids.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Position.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Position_YAngles.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/IInstance_Buffers.hpp>
#include <Mlib/Threads/Safe_Shared_Mutex.hpp>
#include <string>
#include <vector>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct TransformationAndBillboardId;
enum class TransformationMode;
enum class ClearOnUpdate;

class DynamicInstanceBuffers: public IInstanceBuffers {
    DynamicInstanceBuffers(const DynamicInstanceBuffers&) = delete;
    DynamicInstanceBuffers& operator = (const DynamicInstanceBuffers&) = delete;
public:
    DynamicInstanceBuffers(
        TransformationMode transformation_mode,
        size_t max_num_instances,
        uint32_t num_billboard_atlas_components,
        ClearOnUpdate clear_on_update);
    virtual ~DynamicInstanceBuffers() override;

    void append(
        const TransformationMatrix<float, float, 3>& transformation_matrix,
        const BillboardSequence& sequence);
    void move(float dt);
    size_t capacity() const;
    size_t tmp_length() const;
    bool tmp_empty() const;

    // IInstanceBuffers
    virtual bool copy_in_progress() const override;
    virtual void wait() const override;
    virtual void update() override;
    virtual void bind(
        GLuint instance_attribute_index,
        GLuint billboard_ids_attribute_index) const override;
    virtual size_t tmp_num_instances() const override;
    virtual GLsizei num_instances() const override;
private:
    DynamicPositionYAngles position_yangles_;
    DynamicPosition position_;
    DynamicBillboardIds billboard_ids_;
    size_t max_num_instances_;
    uint32_t num_billboard_atlas_components_;
    size_t tmp_num_instances_;
    GLsizei gl_num_instances_;
    TransformationMode transformation_mode_;
    std::vector<float> animation_times_;
    std::vector<const BillboardSequence*> billboard_sequences_;
    ClearOnUpdate clear_on_update_;
    mutable SafeSharedMutex mutex_;
};

}
