#include "Animated_Colored_Vertex_Array_Resource.hpp"

using namespace Mlib;

AnimatedColoredVertexArrayResource::AnimatedColoredVertexArrayResource(const std::shared_ptr<AnimatedColoredVertexArrays>& acvas)
: acvas_{acvas}
{}

AnimatedColoredVertexArrayResource::~AnimatedColoredVertexArrayResource()
{}

std::shared_ptr<AnimatedColoredVertexArrays> AnimatedColoredVertexArrayResource::get_physics_arrays() const {
    return acvas_;
}
