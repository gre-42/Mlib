#include "Dynamic_World.hpp"
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

DynamicWorld::DynamicWorld(const SceneNodeResources& scene_node_resources, VariableAndHash<std::string> name)
    : scene_node_resources_{ scene_node_resources }
    , name_{ std::move(name) }
    , inverse_name_{ *name + ".inverse" }
{}

const TransformationMatrix<double, double, 3>* DynamicWorld::get_geographic_mapping() const
{
    return scene_node_resources_.get_geographic_mapping(name_);
}

const TransformationMatrix<double, double, 3>* DynamicWorld::get_inverse_geographic_mapping() const
{
    return scene_node_resources_.get_geographic_mapping(inverse_name_);
}

const FixedScaledUnitVector<float, 3>* DynamicWorld::get_gravity() const
{
    return scene_node_resources_.get_gravity(name_);
}

const FixedScaledUnitVector<float, 3>* DynamicWorld::get_wind() const {
    return scene_node_resources_.get_wind(name_);
}
