#include "Load_Pssg_Arrays.hpp"
#include <Mlib/Array/Non_Copying_Vector.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Interfaces/IDds_Resources.hpp>
#include <Mlib/Geometry/Material_Configuration/Base_Materials.hpp>
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
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Map/Map.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <algorithm>
#include <half/half.h>
#include <list>
#include <unordered_set>

using namespace Mlib;

static const std::string LOD_LOW_PREFIX = "LOW";
static const auto COLOR_MODE = ColorMode::RGB | ColorMode::RGBA | ColorMode::AGR_NORMAL;

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

enum class ColorSemantic {
    RGBA,
    TEXTURE_WEIGHTS,
    UNUSED
};

struct Shader {
    PhysicsMaterial physics_material = BASE_VISIBLE_TERRAIN_MATERIAL;
    Material render_material;
    ColorSemantic color_semantic = ColorSemantic::RGBA;
};

template <class TInstancePos, class TResourcePos>
void add_instantiables(
    const PssgNode& node,
    const PssgSchema& schema,
    const TransformationMatrix<float, TInstancePos, 3>& m,
    const StringWithHashUnorderedMap<Shader>& shaders,
    const std::string& resource_prefix,
    StringWithHashUnorderedMap<std::shared_ptr<ColoredVertexArray<TResourcePos>>>& resources,
    std::list<InstanceInformation<TInstancePos>>& instances)
{
    TransformationMatrix<float, TInstancePos, 3> trafo{
        node.get_child("TRANSFORM", schema).array<float, 4, 4>().casted<TInstancePos>() };
    auto mc = m * trafo;
    if ((schema.nodes.get(node.type_id).name == "RENDERNODE") &&
        !node.get_attribute("id", schema).string().starts_with(LOD_LOW_PREFIX))
    {
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
                if (auto shader_object = shaders.try_get(VariableAndHash<std::string>{shader.substr(1)}); shader_object != nullptr) {
                    auto& resource = *resources.get(VariableAndHash<std::string>{resource_prefix + indices.substr(1)});
                    if (!resource.material.textures_color.empty()) {
                        THROW_OR_ABORT("Array resource instantiated multiple times");
                    }
                    resource.morphology.physics_material = shader_object->physics_material;
                    resource.material = shader_object->render_material;
                    switch (shader_object->color_semantic) {
                    case ColorSemantic::RGBA:
                        // Do nothing
                        break;
                    case ColorSemantic::TEXTURE_WEIGHTS:
                    case ColorSemantic::UNUSED:
                        for (auto& t : resource.triangles) {
                            for (auto& v : t.flat_iterable()) {
                                v.color = 255;
                            }
                        }
                        resource.alpha.clear();
                        break;
                    default:
                        THROW_OR_ABORT("Unknown color semantic");
                    }
                }
                auto scale = sqrt(sum<0>(squared(mc.R)));
                auto mean_scale = mean(scale);
                if (any(abs(scale - mean_scale) > 1e-3f)) {
                    THROW_OR_ABORT((std::stringstream() << "Scale is anisotropic: " << scale).str());
                }
                auto mcr = TransformationMatrix{ mc.R / mean_scale, mc.t };
                instances.emplace_back(
                    VariableAndHash<std::string>{resource_prefix + indices.substr(1)},
                    mcr,
                    mean_scale,
                    RenderingDynamics::STATIC);
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
                    fixed_full<TPos, 3>((TPos)-42.43),  // position
                    fixed_full<uint8_t, 4>(255),        // color
                    fixed_zeros<float, 2>(),            // uv
                    fixed_zeros<float, 3>(),            // normal
                    fixed_zeros<float, 3>()));          // tangent
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
                offset,                                                                 // src_offset
                stride,                                                                 // src_stride
                (uint32_t)(std::ptrdiff_t)(&cv0->position),                             // dst_offset
                sizeof(ColoredVertex<TPos>),                                            // dst_stride
                element_count,                                                          // nelements
                3,                                                                      // ndim
                size,                                                                   // src_size
                integral_cast<uint32_t>(vertices.size() * sizeof(ColoredVertex<TPos>)), // dst_size
                data.data(),                                                            // src
                (std::byte*)vertices.data(),                                            // dst
                [](float f) { return (TPos)swap_endianness(f); });
        } else if (render_type == "Color") {
            if (any(features & ColoredVertexFeatures::COLOR)) {
                THROW_OR_ABORT("Multiple color atttributes");
            }
            features |= ColoredVertexFeatures::COLOR;
            if (data_type != "uint_color_argb") {
                THROW_OR_ABORT("Unsupported color data type");
            }
            cweight.emplace(element_count);
            strided_copy<uint32_t, FixedArray<float, 4>>(
                offset,                                                                     // src_offset
                stride,                                                                     // src_stride
                0,                                                                          // dst_offset
                sizeof(FixedArray<float, 4>),                                               // dst_stride
                element_count,                                                              // nelements
                1,                                                                          // ndim
                size,                                                                       // src_size
                integral_cast<uint32_t>(cweight->size() * sizeof(FixedArray<float, 4>)),    // dst_size
                data.data(),                                                                // src
                (std::byte*)cweight->data(),                                                // dst
                [](uint32_t f) { return FixedArray<float, 4>{
                    ((f >>  0) & 0xFF) / 255.f,
                    ((f >>  8) & 0xFF) / 255.f,
                    ((f >> 16) & 0xFF) / 255.f,
                    ((f >> 24) & 0xFF) / 255.f
                };});
            // strided_copy<uint32_t, FixedArray<float, 3>>(
            //     offset,                                                     // src_offset
            //     stride,                                                     // src_stride
            //     (uint32_t)(std::ptrdiff_t)(&cv0->color),                    // dst_offset
            //     sizeof(ColoredVertex<TPos>),                                // dst_stride
            //     element_count,                                              // nelements
            //     1,                                                          // ndim
            //     size,                                                       // src_size
            //     vertices.size() * sizeof(ColoredVertex<TPos>),              // dst_size
            //     data.data(),                                                // src
            //     (std::byte*)vertices.data(),                                // dst
            //     [](uint32_t f) { return FixedArray<float, 3>{
            //         ((f >>  8) & 0xFF) / 255.f,
            //         ((f >> 16) & 0xFF) / 255.f,
            //         ((f >> 24) & 0xFF) / 255.f
            //         // 1.f, 1.f, 1.f
            //     };});
            // strided_copy<uint8_t, float>(
            //     offset,                                                     // src_offset
            //     stride,                                                     // src_stride
            //     (uint32_t)(std::ptrdiff_t)(&cv0->color),                    // dst_offset
            //     sizeof(ColoredVertex<TPos>),                                // dst_stride
            //     element_count,                                              // nelements
            //     3,                                                          // ndim
            //     size,                                                       // src_size
            //     vertices.size() * sizeof(ColoredVertex<TPos>),              // dst_size
            //     data.data(),                                                // src
            //     (std::byte*)vertices.data(),                                // dst
            //     [](uint8_t f) { return float(f) / 255; });
        } else if (render_type == "ST") {
            if (any(features & ColoredVertexFeatures::UV)) {
                UUVector<FixedArray<float, 2>> uvx(element_count);
                if ((data_type == "half2") || (data_type == "half4")) {
                    strided_copy<uint16_t, float>(
                        offset,                                                             // src_offset
                        stride,                                                             // src_stride
                        0,                                                                  // dst_offset
                        sizeof(FixedArray<float, 2>),                                       // dst_stride
                        element_count,                                                      // nelements
                        2,                                                                  // ndim
                        size,                                                               // src_size
                        integral_cast<uint32_t>(uvx.size() * sizeof(FixedArray<float, 2>)), // dst_size
                        data.data(),                                                        // src
                        (std::byte*)uvx.data(),                                             // dst
                        [](uint16_t h) { return std::bit_cast<float>(half_to_float(swap_endianness(h))); });
                } else if ((data_type == "float2") || (data_type == "float3") || (data_type == "float4")) {
                    strided_copy<float, float>(
                        offset,                                                             // src_offset
                        stride,                                                             // src_stride
                        0,                                                                  // dst_offset
                        sizeof(FixedArray<float, 2>),                                       // dst_stride
                        element_count,                                                      // nelements
                        2,                                                                  // ndim
                        size,                                                               // src_size
                        integral_cast<uint32_t>(uvx.size() * sizeof(FixedArray<float, 2>)), // dst_size
                        data.data(),                                                        // src
                        (std::byte*)uvx.data(),                                             // dst
                        [](float h) { return swap_endianness(h); });
                } else {
                    THROW_OR_ABORT("Unsupported ST data type: \"" + data_type + '"');
                }
                uv1.push_back(uvx);
            } else {
                features |= ColoredVertexFeatures::UV;
                if ((data_type == "half2") || (data_type == "half4")) {
                    strided_copy<uint16_t, float>(
                        offset,                                                                 // src_offset
                        stride,                                                                 // src_stride
                        (uint32_t)(std::ptrdiff_t)(&cv0->uv),                                   // dst_offset
                        sizeof(ColoredVertex<TPos>),                                            // dst_stride
                        element_count,                                                          // nelements
                        2,                                                                      // ndim
                        size,                                                                   // src_size
                        integral_cast<uint32_t>(vertices.size() * sizeof(ColoredVertex<TPos>)), // dst_size
                        data.data(),                                                            // src
                        (std::byte*)vertices.data(),                                            // dst
                        [](uint16_t h) { return std::bit_cast<float>(half_to_float(swap_endianness(h))); });
                } else if ((data_type == "float2") || (data_type == "float3") || (data_type == "float4")) {
                    strided_copy<float, float>(
                        offset,                                                                 // src_offset
                        stride,                                                                 // src_stride
                        (uint32_t)(std::ptrdiff_t)(&cv0->uv),                                   // dst_offset
                        sizeof(ColoredVertex<TPos>),                                            // dst_stride
                        element_count,                                                          // nelements
                        2,                                                                      // ndim
                        size,                                                                   // src_size
                        integral_cast<uint32_t>(vertices.size() * sizeof(ColoredVertex<TPos>)), // dst_size
                        data.data(),                                                            // src
                        (std::byte*)vertices.data(),                                            // dst
                        [](float h) { return swap_endianness(h); });
                } else {
                    THROW_OR_ABORT("Unsupported ST data type: \"" + data_type + '"');
                }
            }
        } else if (render_type == "Normal") {
            if (any(features & ColoredVertexFeatures::NORMAL)) {
                THROW_OR_ABORT("Vertex has multiple normals");
            }
            features |= ColoredVertexFeatures::NORMAL;
            if (data_type == "half4") {
                strided_copy<uint16_t, float>(
                    offset,                                                                 // src_offset
                    stride,                                                                 // src_stride
                    (uint32_t)(std::ptrdiff_t)(&cv0->normal),                               // dst_offset
                    sizeof(ColoredVertex<TPos>),                                            // dst_stride
                    element_count,                                                          // nelements
                    3,                                                                      // ndim
                    size,                                                                   // src_size
                    integral_cast<uint32_t>(vertices.size() * sizeof(ColoredVertex<TPos>)), // dst_size
                    data.data(),                                                            // src
                    (std::byte*)vertices.data(),                                            // dst
                    [](uint16_t h) { return std::bit_cast<float>(half_to_float(swap_endianness(h))); });
            } else if (data_type == "float3") {
                strided_copy<float, float>(
                    offset,                                                                 // src_offset
                    stride,                                                                 // src_stride
                    (uint32_t)(std::ptrdiff_t)(&cv0->normal),                               // dst_offset
                    sizeof(ColoredVertex<TPos>),                                            // dst_stride
                    element_count,                                                          // nelements
                    3,                                                                      // ndim
                    size,                                                                   // src_size
                    integral_cast<uint32_t>(vertices.size() * sizeof(ColoredVertex<TPos>)), // dst_size
                    data.data(),                                                            // src
                    (std::byte*)vertices.data(),                                            // dst
                    [](float h) { return swap_endianness(h); });
            } else {
                THROW_OR_ABORT("Unsupported Normal data type: \"" + data_type + '"');
            }
        } else if (render_type == "Tangent") {
            if (any(features & ColoredVertexFeatures::TANGENT)) {
                THROW_OR_ABORT("Vertex has multiple tangents");
            }
            features |= ColoredVertexFeatures::TANGENT;
            if (data_type != "half4") {
                THROW_OR_ABORT("Unsupported Tangent data type");
            }
            strided_copy<uint16_t, float>(
                offset,                                                                 // src_offset
                stride,                                                                 // src_stride
                (uint32_t)(std::ptrdiff_t)(&cv0->tangent),                              // dst_offset
                sizeof(ColoredVertex<TPos>),                                            // dst_stride
                element_count,                                                          // nelements
                3,                                                                      // ndim
                size,                                                                   // src_size
                integral_cast<uint32_t>(vertices.size() * sizeof(ColoredVertex<TPos>)), // dst_size
                data.data(),                                                            // src
                (std::byte*)vertices.data(),                                            // dst
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
    std::list<UUVector<FixedArray<float, 2>>> uv1;
    std::optional<UUVector<FixedArray<float, 4>>> cweight;
    ColoredVertexFeatures features = ColoredVertexFeatures::NONE;
};

struct DataBlockStreams {
    const PssgNode* data_block;
    std::vector<const PssgNode*> streams;
};

struct ShaderGroup {
    ShaderGroup()
        : parameters{ "Shader parameter" }
    {}
    StringWithHashUnorderedMap<uint32_t> parameters;
};

template <class TResourcePos, class TInstancePos>
PssgArrays<TResourcePos, TInstancePos> Mlib::load_pssg_arrays(
    const PssgModel& model,
    const LoadMeshConfig<TResourcePos>& cfg,
    IDdsResources* dds_resources,
    const std::string& resource_prefix,
    IoVerbosity verbosity)
{
    using I = funpack_t<TResourcePos>;

    PssgArrays<TResourcePos, TInstancePos> result;
    StringWithHashUnorderedMap<ShaderGroup> shader_groups{ "Shader group" };
    model.root.for_each_node([&](const PssgNode& node){
        const auto& s = model.schema.nodes.get(node.type_id);
        if (s.name == "SHADERGROUP") {
            ShaderGroup sg;
            for (const auto& input_definition : node.children) {
                if (model.schema.nodes.get(input_definition.type_id).name != "SHADERINPUTDEFINITION") {
                    THROW_OR_ABORT("Shader group child is not a shader input definition");
                }
                sg.parameters.add(
                    VariableAndHash<std::string>{input_definition.get_attribute("name", model.schema).string()},
                    integral_cast<uint32_t>(sg.parameters.size()));
            }
            shader_groups.add(
                VariableAndHash<std::string>{node.get_attribute("id", model.schema).string()},
                std::move(sg));
        }
        return true;
    });
    StringWithHashUnorderedMap<DataBlockStreams> data_block_streams{ "Data block stream "};
    model.root.for_each_node([&](const PssgNode& node){
        const auto& s = model.schema.nodes.get(node.type_id);
        if (s.name == "DATABLOCK") {
            auto node_id = VariableAndHash<std::string>{node.get_attribute("id", model.schema).string()};
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
    StringWithHashUnorderedMap<Shader> shaders{ "Shader" };
    model.root.for_each_node([&](const PssgNode& node){
        const auto& s = model.schema.nodes.get(node.type_id);
        if (s.name == "SHADERINSTANCE") {
            auto shader_group_ref = node.get_attribute("shaderGroup", model.schema).string();
            if (!shader_group_ref.starts_with('#')) {
                THROW_OR_ABORT("Shader group reference does not start with \"#\"");
            }
            auto node_id = VariableAndHash<std::string>{node.get_attribute("id", model.schema).string()};
            const auto& shader_group_object = shader_groups.get(VariableAndHash<std::string>{shader_group_ref.substr(1)});
            auto try_get_texture = [&](const VariableAndHash<std::string>& parameter_name) -> std::string {
                uint32_t parameter_id = shader_group_object.parameters.get(parameter_name);
                std::string texture_reference;
                for (const auto& c : node.children) {
                    if ((model.schema.nodes.get(c.type_id).name == "SHADERINPUT") &&
                        (c.get_attribute("parameterID", model.schema).uint32() == parameter_id))
                    {
                        texture_reference = c.get_attribute("texture", model.schema).string();
                        if (texture_reference.starts_with("#")) {
                            return resource_prefix + texture_reference.substr(1) + ".dds";
                        } else {
                            return texture_reference + ".dds";
                        }
                    }
                }
                return "";
                };
            auto get_attribute = [&](const VariableAndHash<std::string>& parameter_name) {
                uint32_t parameter_id = shader_group_object.parameters.get(parameter_name);
                for (const auto& c : node.children) {
                    if ((model.schema.nodes.get(c.type_id).name == "SHADERINPUT") &&
                        (c.get_attribute("parameterID", model.schema).uint32() == parameter_id))
                    {
                        return c;
                    }
                }
                THROW_OR_ABORT("Could not find shader input with parameter name \"" + *parameter_name + '"');
                };
            if ((shader_group_ref == "#terrain_simple.fx") ||
                (shader_group_ref == "#terrain_simple_nm.fx") ||
                (shader_group_ref == "#terrain_road.fx") ||
                (shader_group_ref == "#terrain_road_flat.fx") ||
                (shader_group_ref == "#terrain_edge_nm.fx") ||
                (shader_group_ref == "#terrain_wsm_blend_nm.fx") ||
                (shader_group_ref == "#terrain_wsm_edge_nm.fx") ||
                (shader_group_ref == "#terrain_infield.fx") ||
                (shader_group_ref == "#terrain_infield_nm.fx") ||
                (shader_group_ref == "#terrain_rockbank.fx") ||
                (shader_group_ref == "#terrain_rock_d4.fx"))
            {
                bool infield = (shader_group_ref == "#terrain_infield.fx");
                bool infield_nm = (shader_group_ref == "#terrain_infield_nm.fx");
                bool infield_any = infield || infield_nm;
                bool rock = (shader_group_ref == "#terrain_rock_d4.fx");
                bool simple = (shader_group_ref == "#terrain_simple.fx");
                bool simple_nm = (shader_group_ref == "#terrain_simple_nm.fx");
                bool simple_any = simple || simple_nm;
                uint32_t cweights_a[] = {UINT32_MAX, 2, 1, UINT32_MAX};
                uint32_t cweights_b[] = {UINT32_MAX, 1, 0, 2};
                auto* cweight_ids =
                    infield_any ? cweights_a :
                    cweights_b;
                auto uv_source_mask = infield_any
                    ? BlendMapUvSource::VERTICAL0
                    : BlendMapUvSource::VERTICAL1;
                auto reweight_mode = infield_any
                    ? BlendMapReweightMode::ENABLED
                    : BlendMapReweightMode::DISABLED;
                auto reduction = infield_any
                    ? BlendMapReductionOperation::PLUS
                    : BlendMapReductionOperation::BLEND;

                std::list<BlendMapTexture> textures_color;

                auto blend_uvs = rock
                    ? FixedArray<float, 4>{ 1.f, 1.f, 0.f, 0.f }
                    : get_attribute(VariableAndHash<std::string>{"BlendMaskUVScale"}).template array<float, 4>();
                
                size_t ntextures =
                    simple_any ? 1 :
                    rock ? 3 :
                    4;
                for (size_t i = 1; i <= ntextures; ++i) {
                    std::string s = std::to_string(i);
                    auto op_diffuse = try_get_texture(VariableAndHash<std::string>{"TDiffuseSpecMap" + s});
                    if (op_diffuse.empty()) {
                        continue;
                    }
                    auto op_uvso = rock
                        ? FixedArray<float, 4>{ 1.f, 1.f, 0.f, 0.f }
                        : get_attribute(VariableAndHash<std::string>{"Map" + s + "UVScaleAndOffset"}).template array<float, 4>();

                    auto op_normal = (infield || simple)
                        ? ""
                        : try_get_texture(VariableAndHash<std::string>{"TNormalMap" + s});

                    if (i != 1) {
                        auto blend_map = try_get_texture(VariableAndHash<std::string>{"TBlendMap" + s});
                        if (blend_map.empty()) {
                            lwarn() << "TBlendMap" + s + " texture not specified. Node: " + *node_id;
                            continue;
                        }

                        textures_color.emplace_back(BlendMapTexture{
                            .texture_descriptor = TextureDescriptor{
                                .color = ColormapWithModifiers{
                                    .filename = VariableAndHash{ blend_map },
                                    .color_mode = COLOR_MODE,
                                    .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                    .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                                    .anisotropic_filtering_level = cfg.anisotropic_filtering_level
                                }.compute_hash()
                            },
                            .scale = { blend_uvs(0), blend_uvs(1) },
                            .min_detail_weight = infield_any
                                ? 0.01f
                                : 0.f,
                            .role = BlendMapRole::DETAIL_MASK_R,
                            .uv_source = uv_source_mask,
                            .reduction = BlendMapReductionOperation::TIMES,
                            .reweight_mode = textures_color.empty()
                                ? reweight_mode
                                : BlendMapReweightMode::UNDEFINED
                        });
                    }

                    if (!op_diffuse.empty() &&
                        !op_diffuse.ends_with("#default_d.tga.dds"))
                    {
                        textures_color.emplace_back(BlendMapTexture{
                            .texture_descriptor = TextureDescriptor{
                                .color = ColormapWithModifiers{
                                    .filename = VariableAndHash{ op_diffuse },
                                    .color_mode = COLOR_MODE,
                                    .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                    .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                                    .anisotropic_filtering_level = cfg.anisotropic_filtering_level
                                }.compute_hash(),
                                .normal =
                                    (op_normal.empty() ||
                                    op_normal.ends_with("#default_normal_map_n.tga.dds") ||
                                    op_normal.ends_with("#default_n.tga.dds"))
                                    ? ColormapWithModifiers{}.compute_hash()
                                    : ColormapWithModifiers{
                                        .filename = VariableAndHash{ op_normal },
                                        .color_mode = COLOR_MODE,
                                        .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                        .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                                        .anisotropic_filtering_level = cfg.anisotropic_filtering_level }.compute_hash()
                            },
                            .discreteness = 0,
                            .offset = { op_uvso(2), op_uvso(3) },
                            .scale = { op_uvso(0), op_uvso(1) },
                            .weight = 0.f,
                            .cweight_id = cweight_ids[i - 1],
                            .min_detail_weight = infield_any
                                ? 0.01f
                                : 0.f,
                            .uv_source = BlendMapUvSource::VERTICAL0,
                            .reduction = (i == 1)
                                ? BlendMapReductionOperation::PLUS
                                : reduction,
                            .reweight_mode = textures_color.empty()
                                ? reweight_mode
                                : BlendMapReweightMode::UNDEFINED
                        });
                    }
                }
                shaders.add(
                    node_id,
                    Shader{
                        .render_material = Material{
                            .textures_color = std::vector<BlendMapTexture>(
                                textures_color.begin(),
                                textures_color.end()),
                            .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                            .shading = {
                                .fog_distances = make_orderable(cfg.shading.fog_distances),
                                .fog_ambient = make_orderable(cfg.shading.fog_ambient)}
                        },
                        .color_semantic = ColorSemantic::TEXTURE_WEIGHTS
                    });
            } else if (shader_group_ref == "#terrain_track_vista_d4.fx")
            {
                std::list<BlendMapTexture> textures_color;

                {
                    auto large_colour_map = try_get_texture(VariableAndHash<std::string>{"TlargeColourMap"});
                    if (large_colour_map.empty()) {
                        THROW_OR_ABORT("TlargeColourMap not specified");
                    }
                    textures_color.push_back(BlendMapTexture{
                        .texture_descriptor = TextureDescriptor{
                            .color = ColormapWithModifiers{
                                .filename = VariableAndHash{ large_colour_map },
                                .color_mode = COLOR_MODE,
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                                .anisotropic_filtering_level = cfg.anisotropic_filtering_level
                            }.compute_hash()
                        },
                        .role = BlendMapRole::DETAIL_BASE,
                        .uv_source = BlendMapUvSource::VERTICAL1,
                        .reweight_mode = BlendMapReweightMode::DISABLED});
                }
                {
                    auto base_diffuse = try_get_texture(VariableAndHash<std::string>{"TDiffuseSpecMap1"});
                    if (base_diffuse.empty()) {
                        THROW_OR_ABORT("Diffuse1 texture not specified");
                    }

                    auto base_normal = try_get_texture(VariableAndHash<std::string>{"TNormalMap1"});

                    textures_color.emplace_back(BlendMapTexture{
                        .texture_descriptor = TextureDescriptor{
                            .color = ColormapWithModifiers{
                                .filename = VariableAndHash{ base_diffuse },
                                .color_mode = COLOR_MODE,
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                                .anisotropic_filtering_level = cfg.anisotropic_filtering_level
                            }.compute_hash(),
                            .normal = (base_normal.empty() || (base_normal == "default_normal_map_n.tga.dds"))
                                ? ColormapWithModifiers{}.compute_hash()
                                : ColormapWithModifiers{
                                    .filename = VariableAndHash{ base_normal },
                                    .color_mode = COLOR_MODE,
                                    .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                    .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                                    .anisotropic_filtering_level = cfg.anisotropic_filtering_level }.compute_hash()
                        },
                        .discreteness = 0,
                        .role = BlendMapRole::DETAIL_COLOR,
                        .uv_source = BlendMapUvSource::VERTICAL0
                    });
                }

                for (size_t i = 2; i <= 3; ++i) {
                    std::string s = std::to_string(i);
                    auto op_diffuse = try_get_texture(VariableAndHash<std::string>{"TDiffuseSpecMap" + s});
                    if (op_diffuse.empty()) {
                        continue;
                    }

                    auto blend_map = try_get_texture(VariableAndHash<std::string>{"TBlendMap" + s});
                    if (blend_map.empty()) {
                        lwarn() << "TBlendMap" + s + " texture not specified. Node: " + *node_id;
                        continue;
                    }

                    auto op_normal = try_get_texture(VariableAndHash<std::string>{"TNormalMap" + s});

                    textures_color.emplace_back(BlendMapTexture{
                        .texture_descriptor = TextureDescriptor{
                            .color = ColormapWithModifiers{
                                .filename = VariableAndHash{ blend_map },
                                .color_mode = COLOR_MODE,
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                                .anisotropic_filtering_level = cfg.anisotropic_filtering_level
                            }.compute_hash()
                        },
                        .role = BlendMapRole::DETAIL_MASK_R,
                        .uv_source = BlendMapUvSource::VERTICAL0,
                        .reduction = BlendMapReductionOperation::TIMES
                    });
                    textures_color.emplace_back(BlendMapTexture{
                        .texture_descriptor = TextureDescriptor{
                            .color = ColormapWithModifiers{
                                .filename = VariableAndHash{ op_diffuse },
                                .color_mode = COLOR_MODE,
                                .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                                .anisotropic_filtering_level = cfg.anisotropic_filtering_level
                            }.compute_hash(),
                            .normal = (op_normal.empty() || (op_normal == "default_normal_map_n.tga.dds"))
                                ? ColormapWithModifiers{}.compute_hash()
                                : ColormapWithModifiers{
                                    .filename = VariableAndHash{ op_normal },
                                    .color_mode = COLOR_MODE,
                                    .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                    .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                                    .anisotropic_filtering_level = cfg.anisotropic_filtering_level }.compute_hash()
                        },
                        .discreteness = 0,
                        .weight = 0.f,
                        .role = BlendMapRole::DETAIL_COLOR,
                        .uv_source = BlendMapUvSource::VERTICAL0,
                        .reduction = BlendMapReductionOperation::BLEND
                    });
                }
                shaders.add(
                    node_id,
                    Shader{
                        .render_material = Material{
                            .textures_color = std::vector<BlendMapTexture>(
                                textures_color.begin(),
                                textures_color.end()),
                            .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                            .shading = {
                                .fog_distances = make_orderable(cfg.shading.fog_distances),
                                .fog_ambient = make_orderable(cfg.shading.fog_ambient)}
                        },
                        .color_semantic = ColorSemantic::UNUSED
                    });
            } else if (
                (shader_group_ref == "#decal_ao.fx") ||
                (shader_group_ref == "#decal_ao_vc.fx"))
            {
                auto diffuse = try_get_texture(VariableAndHash<std::string>{"TDiffuseAlphaMap"});
                auto normal = try_get_texture(VariableAndHash<std::string>{"TNormalMap"});
                auto specular = try_get_texture(VariableAndHash<std::string>{"TSpecularMap"});
                if (diffuse.empty()) {
                    THROW_OR_ABORT("TDiffuseAlphaMap texture not specified");
                }
                shaders.add(
                    node_id,
                    Shader{
                        .render_material = Material{
                            .blend_mode = BlendMode::CONTINUOUS,
                            .textures_color = {
                                BlendMapTexture{
                                    .texture_descriptor = TextureDescriptor{
                                        .color = ColormapWithModifiers{
                                            .filename = VariableAndHash{ diffuse },
                                            .color_mode = COLOR_MODE,
                                            .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                            .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                                            .anisotropic_filtering_level = cfg.anisotropic_filtering_level
                                        }.compute_hash(),
                                        .specular = ColormapWithModifiers{
                                            .filename = VariableAndHash{ specular },
                                            .color_mode = COLOR_MODE,
                                            .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                            .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                                            .anisotropic_filtering_level = cfg.anisotropic_filtering_level
                                        }.compute_hash(),
                                        .normal = ColormapWithModifiers{
                                            .filename = VariableAndHash{ normal },
                                            .color_mode = COLOR_MODE,
                                            .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                            .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                                            .anisotropic_filtering_level = cfg.anisotropic_filtering_level
                                        }.compute_hash()
                                    }
                                }
                            },
                            .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLACK_NODE,
                            .shading = {
                                .fog_distances = make_orderable(cfg.shading.fog_distances),
                                .fog_ambient = make_orderable(cfg.shading.fog_ambient)}
                        },
                        .color_semantic = ColorSemantic::RGBA
                    });
            } else if (shader_group_ref == "#terrain_lod.fx")
            {
                auto diffuse = try_get_texture(VariableAndHash<std::string>{"TDiffuseAlphaMap"});
                if (diffuse.empty()) {
                    THROW_OR_ABORT("Diffuse texture not specified");
                }
                shaders.add(
                    node_id,
                    Shader{
                        .render_material = Material{
                            .textures_color = std::vector<BlendMapTexture>{{
                                .texture_descriptor = TextureDescriptor{
                                    .color = ColormapWithModifiers{
                                        .filename = VariableAndHash{ diffuse },
                                        .color_mode = COLOR_MODE,
                                        .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                                        .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                                        .anisotropic_filtering_level = cfg.anisotropic_filtering_level
                                    }.compute_hash()
                                }}},
                            .shading = {
                                .fog_distances = make_orderable(cfg.shading.fog_distances),
                                .fog_ambient = make_orderable(cfg.shading.fog_ambient)}}});
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
                    const auto& dbs = data_block_streams.get(VariableAndHash<std::string>{data_block_name.substr(1)});
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
            std::vector<UUVector<FixedArray<float, 3, 2>>> uv1(dbm.uv1.size());
            for (auto& u : uv1) {
                u.resize(ixs_count / 3);
            }
            std::vector<UUVector<FixedArray<float, 3>>> cweight(dbm.cweight.has_value() ? 3 : 0);
            UUVector<FixedArray<float, 3>> alpha(dbm.cweight.has_value() ? ixs_count / 3 : 0);
            for (auto& c : cweight) {
                c.resize(ixs_count / 3);
            }
            for (uint32_t i = 0; i < ixs_count / 3; ++i) {
                for (uint32_t j = 0; j < 3; ++j) {
                    uint16_t id = swap_endianness(reinterpret_cast<const uint16_t*>(isd.data.data())[i * 3 + j]);
                    if (id >= dbm.vertices.size()) {
                        THROW_OR_ABORT("Vertex index out of bounds: " + std::to_string(id) + " >= " + std::to_string(dbm.vertices.size()));
                    }
                    triangles[i](j) = dbm.vertices[id];
                    for (const auto& [k, u] : enumerate(dbm.uv1)) {
                        uv1[k][i][j] = u[id];
                    }
                    if (dbm.cweight.has_value()) {
                        for (auto&& [k, b] : enumerate(cweight)) {
                            b[i](j) = (*dbm.cweight)[id](k);
                        }
                        alpha[i](j) = (*dbm.cweight)[id](0);
                    }
                }
            }
            auto& cva = *result.resources.add(
                VariableAndHash<std::string>{resource_prefix + node_id},
                std::make_shared<ColoredVertexArray<TResourcePos>>(
                    resource_prefix + node_id,
                    Material{},
                    Morphology{ cfg.physics_material },
                    ModifierBacklog{},
                    UUVector<FixedArray<ColoredVertex<TResourcePos>, 4>>(),
                    std::move(triangles),
                    UUVector<FixedArray<ColoredVertex<TResourcePos>, 2>>(),
                    UUVector<FixedArray<std::vector<BoneWeight>, 3>>(),
                    UUVector<FixedArray<float, 3>>(),
                    UUVector<FixedArray<uint8_t, 3>>(),
                    std::move(uv1),
                    std::move(cweight),
                    std::move(alpha),
                    UUVector<FixedArray<float, 4>>()));
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
                auto te = TriangleTangentErrorBehavior::WARN;
                for (auto& t : cva.triangles) {
                    auto ta = triangle_tangent(
                        funpack(t(0).position),
                        funpack(t(1).position),
                        funpack(t(2).position),
                        t(0).uv.template casted<I>(),
                        t(1).uv.template casted<I>(),
                        t(2).uv.template casted<I>(),
                        te).template casted<float>();
                    for (auto& v : t.flat_iterable()) {
                        v.tangent = ta;
                    }
                    if (all(ta == 0.f)) {
                        te = TriangleTangentErrorBehavior::ZERO;
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
                    .filename = VariableAndHash{ resource_prefix + node_id + ".dds" },
                    .color_mode = COLOR_MODE,
                    .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                    .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                    .anisotropic_filtering_level = cfg.anisotropic_filtering_level
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
        r->material.period_world = cfg.period_world;
        r->material.compute_color_mode();
        r->material.shading.emissive *= cfg.emissive_factor;
        r->material.shading.ambient *= cfg.ambient_factor;
        r->material.shading.diffuse *= cfg.diffuse_factor;
        r->material.shading.specular *= cfg.specular_factor;
    }
    return result;
}

template PssgArrays<float, float> Mlib::load_pssg_arrays<float, float>(
    const PssgModel& model,
    const LoadMeshConfig<float>& cfg,
    IDdsResources* dds_resources,
    const std::string& resource_prefix,
    IoVerbosity verbosity);

template PssgArrays<float, double> Mlib::load_pssg_arrays<float, double>(
    const PssgModel& model,
    const LoadMeshConfig<float>& cfg,
    IDdsResources* dds_resources,
    const std::string& resource_prefix,
    IoVerbosity verbosity);

template PssgArrays<CompressedScenePos, float> Mlib::load_pssg_arrays<CompressedScenePos, float>(
    const PssgModel& model,
    const LoadMeshConfig<CompressedScenePos>& cfg,
    IDdsResources* dds_resources,
    const std::string& resource_prefix,
    IoVerbosity verbosity);

template PssgArrays<CompressedScenePos, double> Mlib::load_pssg_arrays<CompressedScenePos, double>(
    const PssgModel& model,
    const LoadMeshConfig<CompressedScenePos>& cfg,
    IDdsResources* dds_resources,
    const std::string& resource_prefix,
    IoVerbosity verbosity);
