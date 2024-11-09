#include "Load_Pssg_Arrays.hpp"
#include <Mlib/Array/Non_Copying_Vector.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Interfaces/IDds_Resources.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Load/Pssg_Elements.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Mesh/Vertex_Normals.hpp>
#include <Mlib/Geometry/Normal_Vector_Error_Behavior.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Triangle_Tangent.hpp>
#include <Mlib/Images/Flip_Mode.hpp>
#include <Mlib/Io/Endian.hpp>
#include <Mlib/Map/Map.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <algorithm>
#include <half/half.h>

using namespace Mlib;

template <class TSource, class TDestination, class TConvert>
void strided_copy(
    uint32_t src_offset,
    uint32_t src_stride,
    uint32_t dst_offset,
    uint32_t dst_stride,
    uint32_t nelements,
    uint32_t ndim,
    uint32_t src_size,
    uint32_t dst_size,
    const std::byte* src,
    std::byte* dst,
    const TConvert& convert)
{
    if (src_offset > 1000) {
        THROW_OR_ABORT("Source offset too large");
    }
    if (src_stride > 1000) {
        THROW_OR_ABORT("Source stride too large");
    }
    if (dst_offset > 1000) {
        THROW_OR_ABORT("Destination offset too large");
    }
    if (dst_stride > 1000) {
        THROW_OR_ABORT("Destination stride too large");
    }
    if (nelements > 1'000'000'000) {
        THROW_OR_ABORT("Element count too large");
    }
    if (ndim > 1000) {
        THROW_OR_ABORT("Destination stride too large");
    }
    if (src_size > 1'000'000'000) {
        THROW_OR_ABORT("Source size too large");
    }
    if (dst_size > 1'000'000'000) {
        THROW_OR_ABORT("Destination size too large");
    }
    for (uint32_t i = 0; i < nelements; ++i) {
        auto src_j = src_offset + i * src_stride;
        auto dst_j = dst_offset + i * dst_stride;
        if (src_j + ndim * sizeof(TSource) >= src_size + 1) {
            THROW_OR_ABORT("Source data index out of bounds");
        }
        if (dst_j + ndim * sizeof(TDestination) >= dst_size + 1) {
            THROW_OR_ABORT("Destination data index out of bounds");
        }
        for (uint32_t k = 0; k < ndim; ++k) {
            const auto& s = reinterpret_cast<const TSource&>(src[src_j + k * sizeof(TSource)]);
            auto& d = reinterpret_cast<TDestination&>(dst[dst_j + k * sizeof(TDestination)]);
            d = convert(s);
        }
    }
}

struct Shader {
    PhysicsMaterial physics_material =
        PhysicsMaterial::ATTR_VISIBLE |
        PhysicsMaterial::ATTR_COLLIDE |
        PhysicsMaterial::ATTR_CONCAVE;
    Material render_material;
};

template <class TInstancePos, class TResourcePos>
void add_instantiables(
    const PssgNode& node,
    const PssgSchema& schema,
    const TransformationMatrix<float, TInstancePos, 3>& m,
    const Map<std::string, Shader>& shaders,
    const std::string& resource_prefix,
    UnorderedMap<std::string, std::shared_ptr<ColoredVertexArray<TResourcePos>>>& resources,
    std::list<InstanceInformation<TInstancePos>>& instances)
{
    TransformationMatrix<float, TInstancePos, 3> trafo{
        node.get_child("TRANSFORM", schema).array<float, 4, 4>().casted<TInstancePos>() };
    auto mc = m * trafo;
    if (schema.nodes.get(node.type_id).name == "RENDERNODE") {
        node.for_each_node([&](const PssgNode& child){
            if (schema.nodes.get(child.type_id).name == "RENDERSTREAMINSTANCE") {
                auto indices = child.get_attribute("indices", schema).string();
                if (!indices.starts_with('#')) {
                    THROW_OR_ABORT("indices do not start with \"#\"");
                }
                auto shader = child.get_attribute("shader", schema).string();
                if (!shader.starts_with('#')) {
                    THROW_OR_ABORT("shader does not start with \"#\"");
                }
                if (auto s = shaders.try_get(shader.substr(1)); s != nullptr) {
                    auto& resource = *resources.get(resource_prefix + indices.substr(1));
                    const auto& shader_object = shaders.get(shader.substr(1));
                    if (!resource.material.textures_color.empty()) {
                        THROW_OR_ABORT("Array resource instantiated multiple times");
                    }
                    resource.morphology.physics_material = shader_object.physics_material;
                    resource.material = shader_object.render_material;
                }
                auto scale = sqrt(sum<0>(squared(mc.R)));
                auto mean_scale = mean(scale);
                if (any(abs(scale - mean_scale) > 1e-3f)) {
                    THROW_OR_ABORT("Scale is anisotropic");
                }
                auto mcr = TransformationMatrix{ mc.R / mean_scale, mc.t };
                instances.emplace_back(resource_prefix + indices.substr(1), mcr, mean_scale, RenderingDynamics::STATIC);
            }
            return true;
        });
    }
    for (const auto& c : node.children) {
        static const std::set<std::string> CHILDREN {
            "NODE", "RENDERNODE"
        };
        if (CHILDREN.contains(schema.nodes.get(c.type_id).name)) {
            add_instantiables(c, schema, mc, shaders, resource_prefix, resources, instances);
        }
    }
}

template <class TPos>
struct DataBlocks {
    void add(
        const PssgModel& model,
        const PssgNode& data_block,
        const PssgNode& data_block_stream)
    {
        // Process DATABLOCK
        auto element_count = data_block.get_attribute("elementCount", model.schema).uint32();
        auto size = data_block.get_attribute("size", model.schema).uint32();
        if (element_count > 1'000'000'000) {
            THROW_OR_ABORT("Element count too large");
        }
        if (size > 1'000'000'000) {
            THROW_OR_ABORT("Size too large");
        }
        const auto& data = data_block.get_child("DATABLOCKDATA", model.schema).data;
        if (data.size() < size) {
            THROW_OR_ABORT("DATABLOCKDATA too short");
        }
        if (vertices.empty()) {
            vertices.resize(
                element_count,
                ColoredVertex<TPos>(
                    fixed_nans<TPos, 3>(),          // position
                    fixed_ones<float, 3>(),         // color
                    fixed_zeros<float, 2>(),        // uv
                    fixed_zeros<float, 3>(),        // normal
                    fixed_zeros<float, 3>()));      // tangent
        } else if (vertices.size() != element_count) {
            THROW_OR_ABORT("Element count mismatch");
        }
        // Process DATABLOCKSTREAM
        auto offset = data_block_stream.get_attribute("offset", model.schema).uint32();
        auto stride = data_block_stream.get_attribute("stride", model.schema).uint32();
        if (offset > 1000) {
            THROW_OR_ABORT("Offset too large");
        }
        if (stride > 1000) {
            THROW_OR_ABORT("Stride too large");
        }
        auto render_type = data_block_stream.get_attribute("renderType", model.schema).string();
        auto data_type = data_block_stream.get_attribute("dataType", model.schema).string();
        const ColoredVertex<TPos>* cv0 = nullptr;
        if ((render_type == "Vertex") || (render_type == "SkinnableVertex")) {
            features |= ColoredVertexFeatures::POSITION;
            if (data_type != "float3") {
                THROW_OR_ABORT("Unsupported vertex data type");
            }
            strided_copy<float, TPos>(
                offset,                                                     // src_offset
                stride,                                                     // src_stride
                (uint32_t)(std::ptrdiff_t)(&cv0->position),                 // dst_offset
                sizeof(ColoredVertex<TPos>),                                // dst_stride
                element_count,                                              // nelements
                3,                                                          // ndim
                size,                                                       // src_size
                vertices.size() * sizeof(ColoredVertex<TPos>),              // dst_size
                data.data(),                                                // src
                (std::byte*)vertices.data(),                                // dst
                [](float f) { return (TPos)swap_endianness(f); });
        } else if (render_type == "Color") {
            features |= ColoredVertexFeatures::COLOR;
            if (data_type != "uint_color_argb") {
                THROW_OR_ABORT("Unsupported color data type");
            }
            strided_copy<uint8_t, float>(
                offset,                                                     // src_offset
                stride,                                                     // src_stride
                (uint32_t)(std::ptrdiff_t)(&cv0->color),                    // dst_offset
                sizeof(ColoredVertex<TPos>),                                // dst_stride
                element_count,                                              // nelements
                3,                                                          // ndim
                size,                                                       // src_size
                vertices.size() * sizeof(ColoredVertex<TPos>),              // dst_size
                data.data(),                                                // src
                (std::byte*)vertices.data(),                                // dst
                [](uint8_t f) { return float(f) / 255; });
        } else if (render_type == "ST") {
            features |= ColoredVertexFeatures::UV;
            if ((data_type == "half2") || (data_type == "half4")) {
                strided_copy<uint16_t, float>(
                    offset,                                                     // src_offset
                    stride,                                                     // src_stride
                    (uint32_t)(std::ptrdiff_t)(&cv0->uv),                       // dst_offset
                    sizeof(ColoredVertex<TPos>),                                // dst_stride
                    element_count,                                              // nelements
                    2,                                                          // ndim
                    size,                                                       // src_size
                    vertices.size() * sizeof(ColoredVertex<TPos>),              // dst_size
                    data.data(),                                                // src
                    (std::byte*)vertices.data(),                                // dst
                    [](uint16_t h) { return std::bit_cast<float>(half_to_float(swap_endianness(h))); });
            } else if ((data_type == "float2") || (data_type == "float3") || (data_type == "float4")) {
                strided_copy<float, float>(
                    offset,                                                     // src_offset
                    stride,                                                     // src_stride
                    (uint32_t)(std::ptrdiff_t)(&cv0->uv),                       // dst_offset
                    sizeof(ColoredVertex<TPos>),                                // dst_stride
                    element_count,                                              // nelements
                    2,                                                          // ndim
                    size,                                                       // src_size
                    vertices.size() * sizeof(ColoredVertex<TPos>),              // dst_size
                    data.data(),                                                // src
                    (std::byte*)vertices.data(),                                // dst
                    [](float h) { return swap_endianness(h); });
            } else {
                THROW_OR_ABORT("Unsupported ST data type: \"" + data_type + '"');
            }
        } else if (render_type == "Normal") {
            features |= ColoredVertexFeatures::NORMAL;
            if (data_type == "half4") {
                strided_copy<uint16_t, float>(
                    offset,                                                     // src_offset
                    stride,                                                     // src_stride
                    (uint32_t)(std::ptrdiff_t)(&cv0->normal),                   // dst_offset
                    sizeof(ColoredVertex<TPos>),                                // dst_stride
                    element_count,                                              // nelements
                    3,                                                          // ndim
                    size,                                                       // src_size
                    vertices.size() * sizeof(ColoredVertex<TPos>),              // dst_size
                    data.data(),                                                // src
                    (std::byte*)vertices.data(),                                // dst
                    [](uint16_t h) { return std::bit_cast<float>(half_to_float(swap_endianness(h))); });
            } else if (data_type == "float3") {
                strided_copy<float, float>(
                    offset,                                                     // src_offset
                    stride,                                                     // src_stride
                    (uint32_t)(std::ptrdiff_t)(&cv0->normal),                   // dst_offset
                    sizeof(ColoredVertex<TPos>),                                // dst_stride
                    element_count,                                              // nelements
                    3,                                                          // ndim
                    size,                                                       // src_size
                    vertices.size() * sizeof(ColoredVertex<TPos>),              // dst_size
                    data.data(),                                                // src
                    (std::byte*)vertices.data(),                                // dst
                    [](float h) { return swap_endianness(h); });
            } else {
                THROW_OR_ABORT("Unsupported Normal data type: \"" + data_type + '"');
            }
        } else if (render_type == "Tangent") {
            features |= ColoredVertexFeatures::TANGENT;
            if (data_type != "half4") {
                THROW_OR_ABORT("Unsupported Tangent data type");
            }
            strided_copy<uint16_t, float>(
                offset,                                                     // src_offset
                stride,                                                     // src_stride
                (uint32_t)(std::ptrdiff_t)(&cv0->tangent),                  // dst_offset
                sizeof(ColoredVertex<TPos>),                                // dst_stride
                element_count,                                              // nelements
                3,                                                          // ndim
                size,                                                       // src_size
                vertices.size() * sizeof(ColoredVertex<TPos>),              // dst_size
                data.data(),                                                // src
                (std::byte*)vertices.data(),                                // dst
                [](uint16_t h) { return std::bit_cast<float>(half_to_float(swap_endianness(h))); });
        } else if (render_type == "Binormal") {
            // Do nothing
        } else if (render_type == "SkinnableVertex") {
            // Do nothing
        } else if (render_type == "SkinIndices") {
            // Do nothing
        } else if (render_type == "SkinWeights") {
            // Do nothing
        } else if (render_type == "SkinnableNormal") {
            // Do nothing
        } else {
            THROW_OR_ABORT("Unsupported render type: \"" + render_type + '"');
        }
    }
    std::vector<ColoredVertex<TPos>> vertices;
    ColoredVertexFeatures features = ColoredVertexFeatures::NONE;
};

struct DataBlockStreams {
    const PssgNode* data_block;
    std::vector<const PssgNode*> streams;
};

struct ShaderGroup {
    UnorderedMap<std::string, uint32_t> parameters;
};

template <class TResourcePos, class TInstancePos>
PssgArrays<TResourcePos, TInstancePos> Mlib::load_pssg_arrays(
    const PssgModel& model,
    const LoadMeshConfig<TResourcePos>& cfg,
    IDdsResources* dds_resources,
    const std::string& resource_prefix,
    IoVerbosity verbosity)
{
    PssgArrays<TResourcePos, TInstancePos> result;
    UnorderedMap<std::string, ShaderGroup> shader_groups;
    model.root.for_each_node([&](const PssgNode& node){
        const auto& s = model.schema.nodes.get(node.type_id);
        if (s.name == "SHADERGROUP") {
            ShaderGroup sg;
            for (const auto& input_definition : node.children) {
                if (model.schema.nodes.get(input_definition.type_id).name != "SHADERINPUTDEFINITION") {
                    THROW_OR_ABORT("Shader group child is not a shader input definition");
                }
                sg.parameters.add(
                    input_definition.get_attribute("name", model.schema).string(),
                    integral_cast<uint32_t>(sg.parameters.size()));
            }
            shader_groups.add(node.get_attribute("id", model.schema).string(), std::move(sg));
        }
        return true;
    });
    UnorderedMap<std::string, DataBlockStreams> data_block_streams;
    model.root.for_each_node([&](const PssgNode& node){
        const auto& s = model.schema.nodes.get(node.type_id);
        if (s.name == "DATABLOCK") {
            std::string node_id = node.get_attribute("id", model.schema).string();
            uint32_t stream_count = node.get_attribute("streamCount", model.schema).uint32();
            if (stream_count > 100) {
                THROW_OR_ABORT("Stream count too large");
            }
            DataBlockStreams dbs{ &node };
            dbs.streams.reserve(stream_count);
            for (const auto& c : node.children) {
                const auto& cs = model.schema.nodes.get(c.type_id);
                if (cs.name == "DATABLOCKDATA") {
                    continue;
                }
                dbs.streams.push_back(&c);
            }
            if (dbs.streams.size() != stream_count) {
                THROW_OR_ABORT("Stream count mismatch");
            }
            data_block_streams.add(node_id, std::move(dbs));
        }
        return true;
    });
    Map<std::string, Shader> shaders;
    model.root.for_each_node([&](const PssgNode& node){
        const auto& s = model.schema.nodes.get(node.type_id);
        if (s.name == "SHADERINSTANCE") {
            auto shader_group_ref = node.get_attribute("shaderGroup", model.schema).string();
            if (!shader_group_ref.starts_with('#')) {
                THROW_OR_ABORT("Shader group reference does not start with \"#\"");
            }
            auto node_id = node.get_attribute("id", model.schema).string();
            const auto& shader_group_object = shader_groups.get(shader_group_ref.substr(1));
            auto get_texture = [&](const std::string& parameter_name) -> std::string {
                uint32_t parameter_id = shader_group_object.parameters.get(parameter_name);
                std::string texture_reference;
                for (const auto& c : node.children) {
                    if ((model.schema.nodes.get(c.type_id).name == "SHADERINPUT") &&
                        (c.get_attribute("parameterID", model.schema).uint32() == parameter_id))
                    {
                        texture_reference = c.get_attribute("texture", model.schema).string();
                        if (!texture_reference.starts_with("#")) {
                            THROW_OR_ABORT("Texture reference does not start with \"#\": " + texture_reference);
                        }
                        return texture_reference.substr(1) + ".dds";
                    }
                }
                return "";
                };
            if ((shader_group_ref == "#terrain_road.fx") ||
                (shader_group_ref == "#terrain_edge_nm.fx") ||
                (shader_group_ref == "#terrain_wsm_edge_nm.fx"))
            {
                auto diffuse = get_texture("TDiffuseSpecMap1");
                if (diffuse.empty()) {
                    THROW_OR_ABORT("Diffuse texture not specified");
                }
                shaders.add(
                    node_id,
                    Shader{
                        .render_material = Material{
                            .textures_color = std::vector<BlendMapTexture>{{
                                .texture_descriptor = TextureDescriptor{
                                    .color = ColormapWithModifiers{ VariableAndHash{ diffuse } }.compute_hash()
                                }}}}});
            } else if (shader_group_ref == "#terrain_infield_nm.fx") {
                auto diffuse = get_texture("TDiffuseSpecMap2");
                if (diffuse.empty()) {
                    THROW_OR_ABORT("Diffuse texture not specified");
                }
                auto normal = get_texture("TNormalMap2");
                shaders.add(
                    node_id,
                    Shader{
                        .physics_material = PhysicsMaterial::NONE,
                        .render_material = Material{
                            .textures_color = std::vector<BlendMapTexture>{{
                                .texture_descriptor = TextureDescriptor{
                                    .color = ColormapWithModifiers{ VariableAndHash{ diffuse } }.compute_hash(),
                                    .normal = (normal.empty() || (normal == "default_normal_map_n.tga.dds"))
                                        ? ColormapWithModifiers{}.compute_hash()
                                        : ColormapWithModifiers{
                                            .filename = VariableAndHash{ normal },
                                            .color_mode = ColorMode::RGB }.compute_hash()
                                }}}}});
            } else if (shader_group_ref == "#terrain_track_vista_d4.fx")
            {
                auto diffuse = get_texture("TlargeColourMap");
                if (diffuse.empty()) {
                    THROW_OR_ABORT("Diffuse texture not specified");
                }
                shaders.add(
                    node_id,
                    Shader{
                        .render_material = Material{
                            .textures_color = std::vector<BlendMapTexture>{{
                                .texture_descriptor = TextureDescriptor{
                                    .color = ColormapWithModifiers{ VariableAndHash{ diffuse } }.compute_hash()
                                }}}}});
            } else if (shader_group_ref == "#terrain_lod.fx")
            {
                auto diffuse = get_texture("TDiffuseAlphaMap");
                if (diffuse.empty()) {
                    THROW_OR_ABORT("Diffuse texture not specified");
                }
                shaders.add(
                    node_id,
                    Shader{
                        .render_material = Material{
                            .textures_color = std::vector<BlendMapTexture>{{
                                .texture_descriptor = TextureDescriptor{
                                    .color = ColormapWithModifiers{ VariableAndHash{ diffuse } }.compute_hash()
                                }}}}});
            } else if (shader_group_ref == "#batched_track.fx")
            {
                shaders.add(
                    node_id,
                    Shader{
                        .physics_material = PhysicsMaterial::NONE
                    });
            }
        }
        return true;
    });
    model.root.for_each_node([&](const PssgNode& node){
        const auto& s = model.schema.nodes.get(node.type_id);
        if (s.name == "RENDERDATASOURCE") {
            // Load triangle indices
            auto primitive = node.get_attribute("primitive", model.schema).string();
            if (primitive != "triangles") {
                THROW_OR_ABORT("Unsupported render primitive: \"" + primitive + '"');
            }
            const auto& index_source = node.get_child("RENDERINDEXSOURCE", model.schema);
            auto ixs_primitive = index_source.get_attribute("primitive", model.schema).string();
            if (ixs_primitive != "triangles") {
                THROW_OR_ABORT("Unsupported render primitive: \"" + primitive + '"');
            }
            auto ixs_format = index_source.get_attribute("format", model.schema).string();
            if (ixs_format != "ushort") {
                THROW_OR_ABORT("Unsupported triangle format");
            }
            auto ixs_count = index_source.get_attribute("count", model.schema).uint32();
            if (ixs_count > 1'000'000'000) {
                THROW_OR_ABORT("Triangle count too large");
            }
            const auto& isd = index_source.get_child("INDEXSOURCEDATA", model.schema);
            if (isd.data.size() < 2 * ixs_count) {
                THROW_OR_ABORT("Triangle buffer too small");
            }
            if ((ixs_count % 3) != 0) {
                THROW_OR_ABORT("Triangle indices not a multiple of 3");
            }
            DataBlocks<TResourcePos> dbm;
            node.for_each_node([&](const PssgNode& render_stream){
                if (model.schema.nodes.get(render_stream.type_id).name == "RENDERSTREAM") {
                    auto data_block_name = render_stream.get_attribute("dataBlock", model.schema).string();
                    if (!data_block_name.starts_with('#')) {
                        THROW_OR_ABORT("dataBlock does not start with \"#\"");
                    }
                    auto sub_stream = render_stream.get_attribute("subStream", model.schema).uint32();
                    const auto& dbs = data_block_streams.get(data_block_name.substr(1));
                    if (sub_stream >= dbs.streams.size()) {
                        THROW_OR_ABORT("Sub-stream index too large");
                    }
                    dbm.add(model, *dbs.data_block, *dbs.streams[sub_stream]);
                }
                return true;
            });
            auto node_id = node.get_attribute("id", model.schema).string();
            UUVector<FixedArray<ColoredVertex<TResourcePos>, 3>> triangles;
            triangles.resize(ixs_count / 3);
            for (uint32_t i = 0; i < ixs_count / 3; ++i) {
                for (uint32_t j = 0; j < 3; ++j) {
                    uint16_t id = swap_endianness(reinterpret_cast<const uint16_t*>(isd.data.data())[i * 3 + j]);
                    if (id >= dbm.vertices.size()) {
                        THROW_OR_ABORT("Vertex index out of bounds: " + std::to_string(id) + " >= " + std::to_string(dbm.vertices.size()));
                    }
                    triangles[i](j) = dbm.vertices[id];
                }
            }
            auto& cva = *result.resources.add(resource_prefix + node_id, std::make_shared<ColoredVertexArray<TResourcePos>>(
                resource_prefix + node_id,
                Material{},
                Morphology{ cfg.physics_material },
                ModifierBacklog{},
                UUVector<FixedArray<ColoredVertex<TResourcePos>, 4>>(),
                std::move(triangles),
                UUVector<FixedArray<ColoredVertex<TResourcePos>, 2>>(),
                UUVector<FixedArray<std::vector<BoneWeight>, 3>>(),
                UUVector<FixedArray<float, 3>>(),
                UUVector<FixedArray<uint8_t, 3>>()));
            if (!any(dbm.features & ColoredVertexFeatures::POSITION)) {
                THROW_OR_ABORT("Vertices have no position in node \"" + node_id + '"');
            }
            // if (!any(dbm.features & ColoredVertexFeatures::UV)) {
            //     for (auto& t : cva.triangles) {
            //         UvShifter3<TResourcePos> uv3{
            //             512.f,
            //             { t(0).position(0), t(0).position(2) },
            //             { t(1).position(0), t(1).position(2) },
            //             { t(2).position(0), t(2).position(2) },
            //             { WrapMode::REPEAT, WrapMode::REPEAT } };
            //         t(0).uv = uv3.u0;
            //         t(1).uv = uv3.u1;
            //         t(2).uv = uv3.u2;
            //     }
            // }
            if (!any(dbm.features & ColoredVertexFeatures::NORMAL)) {
                VertexNormals<TResourcePos, float> vn;
                for (auto& t : cva.triangles) {
                    vn.add_triangle(t);
                }
                vn.compute_vertex_normals(NormalVectorErrorBehavior::WARN);
                for (auto& t : cva.triangles) {
                    for (auto& v : t.flat_iterable()) {
                        v.normal = vn.get_normal(v.position);
                    }
                }
                // for (auto& t : cva.triangles) {
                //     auto n = triangle_normal<TResourcePos>({
                //         t(0).position,
                //         t(1).position,
                //         t(2).position},
                //         NormalVectorErrorBehavior::WARN).template casted<float>();
                //     for (auto& v : t.flat_iterable()) {
                //         v.normal = n;
                //     }
                // }
            }
            if (!any(dbm.features & ColoredVertexFeatures::TANGENT)) {
                for (auto& t : cva.triangles) {
                    auto ta = triangle_tangent(
                        t(0).position,
                        t(1).position,
                        t(2).position,
                        t(0).uv.template casted<TResourcePos>(),
                        t(1).uv.template casted<TResourcePos>(),
                        t(2).uv.template casted<TResourcePos>(),
                        TriangleTangentErrorBehavior::WARN).template casted<float>();
                    for (auto& v : t.flat_iterable()) {
                        v.tangent = ta;
                    }
                }
            }
        }
        return true;
    });
    model.root.for_each_node([&](const PssgNode& node){
        const auto& s = model.schema.nodes.get(node.type_id);
        if (s.name == "ROOTNODE") {
            add_instantiables(
                node,
                model.schema,
                TransformationMatrix<float, TInstancePos, 3>::identity(),
                shaders,
                resource_prefix,
                result.resources,
                result.instances);
        }
        return true;
    });
    if (dds_resources != nullptr) {
        model.root.for_each_node([&](const PssgNode& node) {
            if (model.schema.nodes.get(node.type_id).name != "TEXTURE") {
                return true;
            }
            auto node_id = node.get_attribute("id", model.schema).string();
            auto width = node.get_attribute("width", model.schema).uint32();
            if (width > 10'000) {
                THROW_OR_ABORT("Width too large");
            }
            auto height = node.get_attribute("height", model.schema).uint32();
            if (height > 10'000) {
                THROW_OR_ABORT("Height too large");
            }
            dds_resources->add_texture(
                ColormapWithModifiers{
                    .filename = VariableAndHash{ node_id + ".dds" }
                }.compute_hash(),
                node.texture(model.schema),
                FlipMode::NONE,
                TextureAlreadyExistsBehavior::WARN);
            return true;
        });
    }
    for (auto& [_, r] : result.resources) {
        if (!cfg.textures.empty()) {
            r->material.textures_color = cfg.textures;
        }
        if (!std::isnan(cfg.period_world)) {
            r->material.period_world = cfg.period_world;
        }
        r->material.compute_color_mode();
        r->material.shading.emissive *= cfg.emissive_factor;
        r->material.shading.ambient *= cfg.ambient_factor;
        r->material.shading.diffuse *= cfg.diffuse_factor;
        r->material.shading.specular *= cfg.specular_factor;
    }
    return result;
}

namespace Mlib {

template PssgArrays<float, float> load_pssg_arrays<float, float>(
    const PssgModel& model,
    const LoadMeshConfig<float>& cfg,
    IDdsResources* dds_resources,
    const std::string& resource_prefix,
    IoVerbosity verbosity);

template PssgArrays<float, double> load_pssg_arrays<float, double>(
    const PssgModel& model,
    const LoadMeshConfig<float>& cfg,
    IDdsResources* dds_resources,
    const std::string& resource_prefix,
    IoVerbosity verbosity);

template PssgArrays<double, float> load_pssg_arrays<double, float>(
    const PssgModel& model,
    const LoadMeshConfig<double>& cfg,
    IDdsResources* dds_resources,
    const std::string& resource_prefix,
    IoVerbosity verbosity);

template PssgArrays<double, double> load_pssg_arrays<double, double>(
    const PssgModel& model,
    const LoadMeshConfig<double>& cfg,
    IDdsResources* dds_resources,
    const std::string& resource_prefix,
    IoVerbosity verbosity);

}
