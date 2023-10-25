#include "Save_Kn5.hpp"
#include <Mlib/Geometry/Mesh/Load/Kn5_Elements.hpp>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

template <class T>
void WriteBinary(std::ostream& str, const T& value, const char* msg) {
    str.write(reinterpret_cast<const char*>(&value), sizeof(T));
    if (str.fail()) {
        THROW_OR_ABORT("Could not write " + std::string(msg) + " to stream");
    }
}

template <class TVec>
static void WriteVector(std::ostream& str, const TVec& vec) {
    str.write(reinterpret_cast<const char*>(vec.data()), integral_cast<std::streamsize>(sizeof(typename TVec::value_type) * vec.size()));
    if (str.fail()) {
        THROW_OR_ABORT("Could not write vector to stream");
    }
}

static void WriteString(std::ostream& str, const std::string& value) {
    WriteVector(str, value);
}

static void WriteStream(std::ostream& ostr, std::istream& istr) {
    ostr << istr.rdbuf();
    if (ostr.fail()) {
        THROW_OR_ABORT("Could not write stream into another");
    }
}

void Mlib::save_kn5(
    const std::string& filename,
    int32_t version,
    std::optional<int32_t> unknownNo,
    const std::map<std::string, kn5Texture>& textures,
    const std::map<size_t, kn5Material>& materials,
    std::istream& nodes)
{
    auto f = create_ofstream(filename, std::ios::binary);

    // Header
    WriteString(*f, "sc6969");
    WriteBinary<int32_t>(*f, version, "version");
    if (unknownNo.has_value()) {
        WriteBinary<int32_t>(*f, unknownNo.value(), "unknownNo");
    }

    // Textures
    WriteBinary<int32_t>(*f, integral_cast<int32_t>(textures.size()), "#textures");
    for (const auto& [n, t] : textures) {
        WriteBinary<int32_t>(*f, t.type, "texture type");
        WriteBinary<uint32_t>(*f, integral_cast<uint32_t>(n.size()), "texture name size");
        WriteVector(*f, n);
        WriteBinary<uint32_t>(*f, integral_cast<uint32_t>(t.data.size()), "texture size");
        WriteVector(*f, t.data);
    }

    // Materials
    WriteBinary<uint32_t>(*f, integral_cast<uint32_t>(materials.size()), "#materials");
    for (const auto& [i, m] : materials)
    {
        WriteBinary<uint32_t>(*f, integral_cast<uint32_t>(m.name.length()), "material name size");
        WriteString(*f, m.name);
        
        WriteBinary<uint32_t>(*f, integral_cast<uint32_t>(m.shader.length()), "material shader size");
        WriteString(*f, m.shader);

        WriteBinary<uint8_t>(*f, (uint8_t)m.blendMode, "blend mode");
        WriteBinary<bool>(*f, m.alphaTested, "alpha-tested");

        if (version > 4) {
            WriteBinary<uint8_t>(*f, (uint8_t)m.depthMode, "depth mode");
            WriteBinary<uint8_t>(*f, (uint8_t)42, "unknown 0");
            WriteBinary<uint8_t>(*f, (uint8_t)43, "unknown 1");
            WriteBinary<uint8_t>(*f, (uint8_t)44, "unknown 2");
        }

        std::map<std::string, float> floats{
            { "ksEmissive", m.ksEmissive },
            { "ksAmbient", m.ksAmbient },
            { "ksDiffuse", m.ksDiffuse },
            { "ksSpecular", m.ksSpecular },
            { "ksSpecularEXP", m.ksSpecularEXP },
            { "ksAlphaRef", m.ksAlphaRef },
            { "diffuseMult", m.diffuseMult },
            { "normalMult", m.normalMult },
            { "useDetail", m.useDetail },
            { "detailUVMultiplier", m.detailUVMultiplier },
            { "multR", m.mult(0) },
            { "multG", m.mult(1) },
            { "multB", m.mult(2) },
            { "multA", m.mult(3) },
            { "detailNMMult", m.detailNMMult },
            { "magicMult", m.magicMult },
            { "fresnelMaxLevel", m.fresnelMaxLevel } };

        WriteBinary<uint32_t>(*f, integral_cast<uint32_t>(floats.size()), "#floats material");

        for (const auto& [n, v] : floats) {
            WriteBinary<uint32_t>(*f, integral_cast<uint32_t>(n.length()), "float name");
            WriteString(*f, n);
            WriteBinary<float>(*f, v, "float value");
        }
        WriteVector(*f, std::vector<uint8_t>(36, (uint8_t)42));

        std::map<std::string, std::string> strings{
            { "txDiffuse",m.txDiffuse },
            { "txNormal",m.txNormal },
            { "txMask",m.txMask},
            { "txDetailR", m.txDetail4(0) },
            { "txDetailG", m.txDetail4(1) },
            { "txDetailB", m.txDetail4(2) },
            { "txDetailA", m.txDetail4(3) },
            { "txDetail", m.txDetail1 },
            { "txDetailNM",m.txDetailNM} };

        WriteBinary<uint32_t>(*f, integral_cast<uint32_t>(strings.size()), "#strings");
        for (const auto& [n, v] : strings) {
            if (v.empty()) {
                continue;
            }
            WriteBinary<uint32_t>(*f, integral_cast<uint32_t>(n.length()), "string name");
            WriteString(*f, n);

            WriteBinary<uint32_t>(*f, integral_cast<uint32_t>(n.length()), "string value");
            WriteString(*f, n);
        }
    }

    // Nodes
    WriteStream(*f, nodes);
}
