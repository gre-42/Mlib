#include "Load_Pssg_Arrays.hpp"
#include <Mlib/Array/Non_Copying_Vector.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Interfaces/IDds_Resources.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Pssg.hpp>
#include <Mlib/Geometry/Mesh/Load/Pssg_Elements.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Normal_Vector_Error_Behavior.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Io/Endian.hpp>
#include <Mlib/Map/Map.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <algorithm>
#include <half/half.h>

using namespace Mlib;

template <class TResourcePos, class TInstancePos>
PssgArrays<TResourcePos, TInstancePos> Mlib::load_pssg_arrays(
    const std::string& filename,
    const LoadMeshConfig<TResourcePos>& cfg,
    IDdsResources* dds_resources)
{
    auto ifs = create_ifstream(filename, std::ios::binary);
    if (ifs->fail()) {
        THROW_OR_ABORT("Could not open file for read: \"" + filename + '"');
    }
    try {
        return load_pssg_arrays<TResourcePos, TInstancePos>(
            *ifs,
            filename,
            cfg,
            dds_resources);
    } catch (std::runtime_error& e) {
        THROW_OR_ABORT("Could not read file \"" + filename + "\": "+ e.what());
    }
}

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

template <class TInstancePos, class TResourcePos>
void add_instantiables(
    const PssgNode& node,
    const PssgSchema& schema,
    const TransformationMatrix<float, TInstancePos, 3>& m,
    const Map<std::string, std::vector<BlendMapTexture>>& shaders,
    UnorderedMap<std::string, std::shared_ptr<ColoredVertexArray<TResourcePos>>>& resources,
    std::list<InstanceInformation<TInstancePos>>& instances)
{
    TransformationMatrix<float, TInstancePos, 3> trafo{
        node.get_child("TRANSFORM", schema).smat4x4().casted<TInstancePos>() };
    auto mc = m * trafo;
    if (schema.nodes.get(node.type_id).name == "RENDERNODE") {
        const auto& rsi = node.get_child("RENDERSTREAMINSTANCE", schema);
        auto indices = rsi.get_attribute("indices", schema).string();
        if (!indices.starts_with('#')) {
            THROW_OR_ABORT("indices do not start with \"#\"");
        }
        auto shader = rsi.get_attribute("shader", schema).string();
        if (!shader.starts_with('#')) {
            THROW_OR_ABORT("shader does not start with \"#\"");
        }
        if (auto s = shaders.try_get(shader.substr(1)); s != nullptr) {
            resources.get(indices.substr(1))->material.textures_color = shaders.get(shader.substr(1));
        }
        instances.emplace_back(indices.substr(1), mc, 1.f, RenderingDynamics::STATIC);
    }
    for (const auto& c : node.children) {
        static const std::set<std::string> CHILDREN {
            "NODE", "RENDERNODE"
        };
        if (CHILDREN.contains(schema.nodes.get(c.type_id).name)) {
            add_instantiables(c, schema, mc, shaders, resources, instances);
        }
    }
}

template <class TResourcePos, class TInstancePos>
PssgArrays<TResourcePos, TInstancePos> Mlib::load_pssg_arrays(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<TResourcePos>& cfg,
    IDdsResources* dds_resources)
{
    PssgArrays<TResourcePos, TInstancePos> result;
    auto pssg = load_pssg(istr, name, std::numeric_limits<std::streamoff>::max(), IoVerbosity::SILENT);
    UnorderedMap<std::string, std::vector<ColoredVertex<TResourcePos>>> data_blocks;
    pssg.root.for_each_node([&](const PssgNode& node){
        const auto& s = pssg.schema.nodes.get(node.type_id);
        if (s.name == "DATABLOCK") {
            // Load vertices, color, ST?, ...
            auto element_count = node.get_attribute("elementCount", pssg.schema).uint32();
            auto size = node.get_attribute("size", pssg.schema).uint32();
            if (element_count > 1'000'000'000) {
                THROW_OR_ABORT("Element count too large");
            }
            if (size > 1'000'000'000) {
                THROW_OR_ABORT("Size too large");
            }
            const auto& data = node.get_child("DATABLOCKDATA", pssg.schema).data;
            if (data.size() < size) {
                THROW_OR_ABORT("DATABLOCKDATA too short");
            }
            auto& b = data_blocks.add(
                node.get_attribute("id", pssg.schema).string(),
                element_count,
                ColoredVertex<TResourcePos>(
                    fixed_nans<TResourcePos, 3>(), // position
                    fixed_ones<float, 3>(),        // color
                    fixed_zeros<float, 2>(),       // uv
                    fixed_zeros<float, 3>(),       // normal
                    fixed_zeros<float, 3>()));     // tangent
            for (const auto& c : node.children) {
                const auto& cs = pssg.schema.nodes.get(c.type_id);
                if (cs.name == "DATABLOCKDATA") {
                    continue;
                }
                auto offset = c.get_attribute("offset", pssg.schema).uint32();
                auto stride = c.get_attribute("stride", pssg.schema).uint32();
                if (offset > 1000) {
                    THROW_OR_ABORT("Offset too large");
                }
                if (stride > 1000) {
                    THROW_OR_ABORT("Stride too large");
                }
                auto render_type = c.get_attribute("renderType", pssg.schema).string();
                auto data_type = c.get_attribute("dataType", pssg.schema).string();
                const ColoredVertex<TResourcePos>* cv0 = nullptr;
                if (render_type == "Vertex") {
                    if (data_type != "float3") {
                        THROW_OR_ABORT("Unsupported vertex data type");
                    }
                    strided_copy<float, TResourcePos>(
                        offset,                                         // src_offset
                        stride,                                         // src_stride
                        (uint32_t)(std::ptrdiff_t)(&cv0->position),     // dst_offset
                        sizeof(ColoredVertex<TResourcePos>),            // dst_stride
                        element_count,                                  // nelements
                        3,                                              // ndim
                        size,                                           // src_size
                        b.size() * sizeof(ColoredVertex<TResourcePos>), // dst_size
                        data.data(),                                    // src
                        (std::byte*)b.data(),                           // dst
                        [](float f) { return (TResourcePos)swap_endianness(f); });
                } else if (render_type == "Color") {
                    if (data_type != "uint_color_argb") {
                        THROW_OR_ABORT("Unsupported color data type");
                    }
                    strided_copy<uint8_t, float>(
                        offset + 1,                                     // src_offset. Add "1" to skip the unsupported "a" component
                        stride,                                         // src_stride
                        (uint32_t)(std::ptrdiff_t)(&cv0->color),        // dst_offset
                        sizeof(ColoredVertex<TResourcePos>),            // dst_stride
                        element_count,                                  // nelements
                        3,                                              // ndim
                        size,                                           // src_size
                        b.size() * sizeof(ColoredVertex<TResourcePos>), // dst_size
                        data.data(),                                    // src
                        (std::byte*)b.data(),                           // dst
                        [](uint8_t f) { return float(f) / 255; });
                } else if (render_type == "ST") {
                    if ((data_type == "half4") || (data_type == "float4")) {
                        continue;
                    }
                    if (data_type != "half2") {
                        THROW_OR_ABORT("Unsupported ST data type: \"" + data_type + '"');
                    }
                    strided_copy<uint16_t, float>(
                        offset,                                         // src_offset
                        stride,                                         // src_stride
                        (uint32_t)(std::ptrdiff_t)(&cv0->uv),           // dst_offset
                        sizeof(ColoredVertex<TResourcePos>),            // dst_stride
                        element_count,                                  // nelements
                        2,                                              // ndim
                        size,                                           // src_size
                        b.size() * sizeof(ColoredVertex<TResourcePos>), // dst_size
                        data.data(),                                    // src
                        (std::byte*)b.data(),                           // dst
                        [](uint16_t h) { return std::bit_cast<float>(half_to_float(swap_endianness(h))); });
                } else if (render_type == "Normal") {
                    if (data_type != "half4") {
                        THROW_OR_ABORT("Unsupported Normal data type");
                    }
                    strided_copy<uint16_t, float>(
                        offset,                                         // src_offset
                        stride,                                         // src_stride
                        (uint32_t)(std::ptrdiff_t)(&cv0->normal),       // dst_offset
                        sizeof(ColoredVertex<TResourcePos>),            // dst_stride
                        element_count,                                  // nelements
                        3,                                              // ndim
                        size,                                           // src_size
                        b.size() * sizeof(ColoredVertex<TResourcePos>), // dst_size
                        data.data(),                                    // src
                        (std::byte*)b.data(),                           // dst
                        [](uint16_t h) { return std::bit_cast<float>(half_to_float(swap_endianness(h))); });
                } else if (render_type == "Tangent") {
                    if (data_type != "half4") {
                        THROW_OR_ABORT("Unsupported Tangent data type");
                    }
                    strided_copy<uint16_t, float>(
                        offset,                                         // src_offset
                        stride,                                         // src_stride
                        (uint32_t)(std::ptrdiff_t)(&cv0->tangent),      // dst_offset
                        sizeof(ColoredVertex<TResourcePos>),            // dst_stride
                        element_count,                                  // nelements
                        3,                                              // ndim
                        size,                                           // src_size
                        b.size() * sizeof(ColoredVertex<TResourcePos>), // dst_size
                        data.data(),                                    // src
                        (std::byte*)b.data(),                           // dst
                        [](uint16_t h) { return std::bit_cast<float>(half_to_float(swap_endianness(h))); });
                } else if (render_type == "Binormal") {
                    // Do nothing
                } else {
                    THROW_OR_ABORT("Unsupported render type: \"" + cs.name + '"');
                }
            }
        }
        return true;
    });
    Map<std::string, std::vector<BlendMapTexture>> shaders;
    pssg.root.for_each_node([&](const PssgNode& node){
        const auto& s = pssg.schema.nodes.get(node.type_id);
        if (s.name == "SHADERINSTANCE") {
            auto shader_group = node.get_attribute("shaderGroup", pssg.schema).string();
            auto node_id = node.get_attribute("id", pssg.schema).string();
            if (shader_group == "#terrain_road.fx") {
                auto diffuse = [&]() -> std::string {
                    for (const auto& c : node.children) {
                        if ((pssg.schema.nodes.get(c.type_id).name == "SHADERINPUT") &&
                            (c.get_attribute("parameterID", pssg.schema).uint32() == 11))
                        {
                            return c.get_attribute("texture", pssg.schema).string();
                        }
                    }
                    return "";
                    }();
                if (!diffuse.starts_with("#")) {
                    THROW_OR_ABORT("Diffuse does not start with \"#\": " + diffuse);
                }
                shaders.add(
                    node_id,
                    std::vector<BlendMapTexture>{{
                        .texture_descriptor = TextureDescriptor{
                            .color = ColormapWithModifiers{ VariableAndHash{ diffuse.substr(1) + ".dds" } }.compute_hash()
                        }}});
            }
        }
        return true;
    });
    pssg.root.for_each_node([&](const PssgNode& node){
        const auto& s = pssg.schema.nodes.get(node.type_id);
        if (s.name == "RENDERDATASOURCE") {
            // Load triangle indices
            auto primitive = node.get_attribute("primitive", pssg.schema).string();
            if (primitive != "triangles") {
                THROW_OR_ABORT("Unsupported render primitive: \"" + primitive + '"');
            }
            const auto& index_source = node.get_child("RENDERINDEXSOURCE", pssg.schema);
            auto ixs_primitive = index_source.get_attribute("primitive", pssg.schema).string();
            if (ixs_primitive != "triangles") {
                THROW_OR_ABORT("Unsupported render primitive: \"" + primitive + '"');
            }
            auto ixs_format = index_source.get_attribute("format", pssg.schema).string();
            if (ixs_format != "ushort") {
                THROW_OR_ABORT("Unsupported triangle format");
            }
            auto ixs_count = index_source.get_attribute("count", pssg.schema).uint32();
            if (ixs_count > 1'000'000'000) {
                THROW_OR_ABORT("Triangle count too large");
            }
            const auto& isd = index_source.get_child("INDEXSOURCEDATA", pssg.schema);
            if (isd.data.size() < 2 * ixs_count) {
                THROW_OR_ABORT("Triangle buffer too small");
            }
            if ((ixs_count % 3) != 0) {
                THROW_OR_ABORT("Triangle indices not a multiple of 3");
            }
            const auto& render_stream = node.get_child("RENDERSTREAM", pssg.schema);
            auto data_block_name = render_stream.get_attribute("dataBlock", pssg.schema).string();
            if (!data_block_name.starts_with('#')) {
                THROW_OR_ABORT("dataBlock does not start with \"#\"");
            }
            const auto& data_block = data_blocks.get(data_block_name.substr(1));
            auto node_id = node.get_attribute("id", pssg.schema).string();
            UUVector<FixedArray<ColoredVertex<TResourcePos>, 3>> triangles;
            triangles.resize(ixs_count / 3);
            for (uint32_t i = 0; i < ixs_count / 3; ++i) {
                for (uint32_t j = 0; j < 3; ++j) {
                    uint16_t id = swap_endianness(reinterpret_cast<const uint16_t*>(isd.data.data())[i * 3 + j]);
                    if (id >= data_block.size()) {
                        THROW_OR_ABORT("Vertex index out of bounds: " + std::to_string(id) + " >= " + std::to_string(data_block.size()));
                    }
                    triangles[i](j) = data_block[id];
                }
            }
            result.resources.add(node_id, std::make_shared<ColoredVertexArray<TResourcePos>>(
                node_id,
                Material{},
                Morphology{ cfg.physics_material },
                ModifierBacklog{},
                UUVector<FixedArray<ColoredVertex<TResourcePos>, 4>>(),
                std::move(triangles),
                UUVector<FixedArray<ColoredVertex<TResourcePos>, 2>>(),
                UUVector<FixedArray<std::vector<BoneWeight>, 3>>(),
                UUVector<FixedArray<float, 3>>(),
                UUVector<FixedArray<uint8_t, 3>>()));
        }
        return true;
    });
    pssg.root.for_each_node([&](const PssgNode& node){
        const auto& s = pssg.schema.nodes.get(node.type_id);
        if (s.name == "ROOTNODE") {
            add_instantiables(
                node,
                pssg.schema,
                TransformationMatrix<float, TInstancePos, 3>::identity(),
                shaders,
                result.resources,
                result.instances);
        }
        return true;
    });
    if (dds_resources != nullptr) {
        pssg.root.for_each_node([&](const PssgNode& node) {
            if (pssg.schema.nodes.get(node.type_id).name != "TEXTURE") {
                return true;
            }
            auto node_id = node.get_attribute("id", pssg.schema).string();
            auto width = node.get_attribute("width", pssg.schema).uint32();
            if (width > 10'000) {
                THROW_OR_ABORT("Width too large");
            }
            auto height = node.get_attribute("height", pssg.schema).uint32();
            if (height > 10'000) {
                THROW_OR_ABORT("Height too large");
            }
            dds_resources->add_texture(
                ColormapWithModifiers{
                    .filename = VariableAndHash{ node_id + ".dds" }
                }.compute_hash(),
                node.texture(pssg.schema),
                TextureAlreadyExistsBehavior::RAISE);
            return true;
        });
    }
    for (auto& [_, r] : result.resources) {
        r->material.compute_color_mode();
    }
    return result;
}

namespace Mlib {

template PssgArrays<float, ScenePos> load_pssg_arrays<float>(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg,
    IDdsResources* dds_resources);

template PssgArrays<double, ScenePos> load_pssg_arrays<double>(
    const std::string& filename,
    const LoadMeshConfig<double>& cfg,
    IDdsResources* dds_resources);

template PssgArrays<float, ScenePos> load_pssg_arrays<float>(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<float>& cfg,
    IDdsResources* dds_resources);

template PssgArrays<double, ScenePos> load_pssg_arrays<double>(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<double>& cfg,
    IDdsResources* dds_resources);

}
