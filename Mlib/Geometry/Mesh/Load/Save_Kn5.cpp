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
    std::istream& materials_and_nodes)
{
    auto f = create_ofstream(filename, std::ios::binary);
    WriteString(*f, "sc6969");
    WriteBinary<int32_t>(*f, version, "version");
    if (unknownNo.has_value()) {
        WriteBinary<int32_t>(*f, unknownNo.value(), "unknownNo");
    }
    WriteBinary<int32_t>(*f, integral_cast<int32_t>(textures.size()), "#textures");
    for (const auto& [n, t] : textures) {
        WriteBinary<int32_t>(*f, t.type, "texture type");
        WriteBinary<uint32_t>(*f, integral_cast<uint32_t>(n.size()), "texture name size");
        WriteVector(*f, n);
        WriteBinary<uint32_t>(*f, integral_cast<uint32_t>(t.data.size()), "texture size");
        WriteVector(*f, t.data);
    }
    WriteStream(*f, materials_and_nodes);
}
