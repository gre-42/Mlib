#pragma once
#include <list>

namespace Mlib {

class RoadPropertiesTriangleList;
struct StreetRectangle;
class SceneNodeResources;

void draw_into_street_rectangles(
    RoadPropertiesTriangleList& tl_street,
    std::list<StreetRectangle>& street_rectangles,
    SceneNodeResources& scene_node_resources,
    float height,
    float scale);

}
