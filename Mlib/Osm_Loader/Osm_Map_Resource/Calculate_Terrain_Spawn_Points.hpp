#pragma once
#include <cstddef>
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct SpawnPoint;
struct Building;
struct Node;
class GroundBvh;
class Sample_SoloMesh;
template <typename TData, size_t... tshape>
class FixedArray;
template <class T>
struct Bijection;

void calculate_terrain_spawn_points(
    std::list<SpawnPoint>& spawn_points,
    const std::list<Building>& spawn_lines,
    const std::map<std::string, Node>& nodes,
    const GroundBvh& ground_bvh,
    const Bijection<FixedArray<double, 3, 3>>* to_meters,
    const Sample_SoloMesh* ssm);

}
