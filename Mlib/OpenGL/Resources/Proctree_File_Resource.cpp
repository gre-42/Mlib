#include "Proctree_File_Resource.hpp"
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Json/Misc.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource.hpp>
#include <proctree/proctree.hpp>
#include <proctree/proctree_array.hpp>
#include <proctree/properties_and_seed.hpp>
#include <proctree/properties_and_seed_json.hpp>
#include <stdexcept>

using namespace Mlib;

ProctreeFileResource::ProctreeFileResource(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg)
    : ISceneNodeResource{"ProctreeFileResource"}
{
    Proctree::Tree tree;
    Proctree::PropertiesAndSeed ps;
    {
        auto f = create_ifstream(filename);
        if (f->fail()) {
            throw std::runtime_error("Could not open for read: \"" + filename + '"');
        }
        nlohmann::json j;
        *f >> j;
        if (f->fail()) {
            throw std::runtime_error("Could not for read: \"" + filename + '"');
        }
        ps = j.get<Proctree::PropertiesAndSeed>();
        tree.mProperties = ps.properties;
        if (ps.seed == 0) {
            throw std::runtime_error("seed=0 not allowed");
        }
        srand(ps.seed);
    }
    tree.generate();
    auto& primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
    auto trunk_color = ColormapWithModifiers{
        .filename = FPath{ps.trunk_diffuse},
        .color_mode = ColorMode::RGB,
        .mipmap_mode = MipmapMode::WITH_MIPMAPS,
        .magnifying_interpolation_mode = InterpolationMode::LINEAR,
        .anisotropic_filtering_level = cfg.anisotropic_filtering_level}.compute_hash();
    auto twig_color = ColormapWithModifiers{
        .filename = FPath{ps.twig_diffuse},
        .color_mode = ColorMode::RGBA,
        .mipmap_mode = MipmapMode::WITH_MIPMAPS,
        .magnifying_interpolation_mode = InterpolationMode::LINEAR,
        .anisotropic_filtering_level = cfg.anisotropic_filtering_level}.compute_hash();
    auto trunk_material = Material{
        .textures_color = {BlendMapTexture{
            .texture_descriptor = {
                .color = primary_rendering_resources.colormap(trunk_color)}}}};
    auto twig_material = Material{
        .blend_mode = BlendMode::CONTINUOUS,
        .textures_color = {BlendMapTexture{
            .texture_descriptor = {
                .color = primary_rendering_resources.colormap(twig_color)}}}};
    auto cvas = proctree_to_cvas(tree, trunk_material, twig_material);
    rva_ = std::make_shared<ColoredVertexArrayResource>(cvas);
}

ProctreeFileResource::~ProctreeFileResource() = default;

void ProctreeFileResource::preload(const RenderableResourceFilter& filter) const {
    rva_->preload(filter);
}

void ProctreeFileResource::instantiate_child_renderable(const ChildInstantiationOptions& options) const
{
    rva_->instantiate_child_renderable(options);
}

void ProctreeFileResource::instantiate_root_renderables(const RootInstantiationOptions& options) const
{
    rva_->instantiate_root_renderables(options);
}

AggregateMode ProctreeFileResource::get_aggregate_mode() const {
    return rva_->get_aggregate_mode();
}
