#include "Add_Trees_To_Zonemap.hpp"
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Bounding_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Compute_Area.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Ground_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Resource_Name_Cycle.hpp>
#include <Mlib/Scene_Graph/Resources/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>

using namespace Mlib;

void Mlib::add_trees_to_zonemap(
    BatchResourceInstantiator& bri,
    ResourceNameCycle& rnc,
    const BoundingInfo& bounding_info,
    double min_dist_to_road,
    const StreetBvh& street_bvh,
    const GroundBvh& ground_bvh,
    const Array<double>& tree_density,
    double tree_density_width,
    double tree_density_height,
    double tree_density_multiplier,
    float jitter,
    double step_size,
    double position_scale,
    CompressedScenePos min_height)
{
    FastUniformRandomNumberGenerator<double> prob_rng{ 0 };
    FastNormalRandomNumberGenerator<float> scale_rng{ 0, 1.f, 0.2f };
    FastNormalRandomNumberGenerator<float> jitter_rng{ 0, 0.f, jitter };
    for (CompressedScenePos x = bounding_info.boundary_min(0); x < bounding_info.boundary_max(0); x += (CompressedScenePos)(step_size * position_scale)) {
        for (CompressedScenePos y = bounding_info.boundary_min(1); y < bounding_info.boundary_max(1); y += (CompressedScenePos)(step_size * position_scale)) {
            FixedArray<CompressedScenePos, 2> pos{
                x + (CompressedScenePos)(position_scale * (double)jitter_rng()),
                y + (CompressedScenePos)(position_scale * (double)jitter_rng()) };
            FixedArray<double, 2> size{
                tree_density_width * position_scale,
                tree_density_height * position_scale};
            FixedArray<double, 2> uv = funpack(pos) / size;
            uv(0) -= std::floor(uv(0));
            uv(1) -= std::floor(uv(1));
            double prob;
            if (!bilinear_grayscale_interpolation(
                uv(1) * double(tree_density.shape(0) - 1),
                uv(0) * double(tree_density.shape(1) - 1),
                tree_density,
                prob))
            {
                continue;
            }
            if (prob_rng() > prob * tree_density_multiplier) {
                continue;
            }
            if (std::isnan(min_dist_to_road) || !street_bvh.has_neighbor(pos, (CompressedScenePos)(min_dist_to_road * position_scale))) {
                CompressedScenePos height;
                if (ground_bvh.max_height(height, pos) && (height > min_height * position_scale)) {
                    if (auto prn = rnc.try_multiple_times(10); prn != nullptr) {
                        bri.add_parsed_resource_name(pos, height, *prn, 0.f, scale_rng());
                    }
                }
            }
        }
    }
}
