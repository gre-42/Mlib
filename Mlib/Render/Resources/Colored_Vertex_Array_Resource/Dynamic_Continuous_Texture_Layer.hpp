#pragma once
#include <Mlib/Render/Instance_Handles/IArray_Buffer.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Base.hpp>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

class DynamicContinuousTextureLayer: public IArrayBuffer {
    DynamicContinuousTextureLayer(const DynamicContinuousTextureLayer&) = delete;
    DynamicContinuousTextureLayer& operator = (const DynamicContinuousTextureLayer&) = delete;
public:
    explicit DynamicContinuousTextureLayer(size_t max_num_triangles);
    ~DynamicContinuousTextureLayer();

    virtual bool copy_in_progress() const override;
    virtual void wait() const override;
    virtual void update() override;
    virtual void bind() const override;
    virtual bool is_awaited() const override;
    virtual std::shared_ptr<IArrayBuffer> fork() override;

    void append(const FixedArray<float, 3>& layers);
    FixedArray<float, 3>& operator [] (size_t i);
    void remove(size_t i);
protected:
    virtual void set_type_erased(
        const char* begin,
        const char* end,
        TaskLocation task_location) override;

private:
    DynamicBase<FixedArray<float, 3>> data_;
};

}
