#pragma once
#include <Mlib/Variable_And_Hash.hpp>
#include <chrono>
#include <cstddef>
#include <string>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TData, size_t n>
struct FixedScaledUnitVector;

class SceneNodeResources;

class DynamicWorld {
    DynamicWorld(const DynamicWorld&) = delete;
    DynamicWorld& operator = (const DynamicWorld&) = delete;
public:
    DynamicWorld(const SceneNodeResources& scene_node_resources, VariableAndHash<std::string> name);

    const TransformationMatrix<double, double, 3>* get_geographic_mapping() const;
    const TransformationMatrix<double, double, 3>* get_inverse_geographic_mapping() const;
    const FixedScaledUnitVector<float, 3>* get_gravity() const;
    const FixedScaledUnitVector<float, 3>* get_wind() const;
private:
    const SceneNodeResources& scene_node_resources_;
    VariableAndHash<std::string> name_;
    VariableAndHash<std::string> inverse_name_;
};

}
