#pragma once
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Ptr.hpp>
#include <Mlib/Scene_Graph/Interfaces/IDynamic_Lights.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <chrono>
#include <cstddef>
#include <functional>
#include <unordered_set>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class DynamicLightDb;
class IDynamicLight;

class DynamicLights: public IDynamicLights {
public:
    explicit DynamicLights(const DynamicLightDb& db);
    virtual ~DynamicLights() override;
    std::unique_ptr<IDynamicLight> instantiate(
        const std::string& name,
        const std::function<FixedArray<ScenePos, 3>()>& get_position,
        std::chrono::steady_clock::time_point time,
        SourceLocation loc);
    void erase(IDynamicLight& light);

    // IDynamicLights
    virtual bool empty() const override;
    virtual void append_time(std::chrono::steady_clock::time_point time) override;
    virtual void set_time(std::chrono::steady_clock::time_point time) override;
    virtual FixedArray<float, 3> get_color(const FixedArray<ScenePos, 3>& target_position) const override;

private:
    const DynamicLightDb& db_;
    std::unordered_set<
        DanglingBaseClassPtr<IDynamicLight>,
        std::hash<DanglingBaseClassPtr<IDynamicLight>>,
        DestructionFunctionsTokensPtrComparator<IDynamicLight>> instances_;
    mutable FastMutex mutex_;
};

}
