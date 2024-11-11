#include "Pssg_Elements.hpp"
#include <Mlib/Images/Dds_Header.hpp>
#include <Mlib/Io/Endian.hpp>
#include <map>

using namespace Mlib;

std::string PssgAttribute::string() const {
    if (data.size() < 4) {
        THROW_OR_ABORT("PSSG string attribute too short");
    }
    auto length = swap_endianness(*reinterpret_cast<const uint32_t*>(data.data()));
    if (length != data.size() - 4) {
        THROW_OR_ABORT("PSSG string attribute size mismatch");
    }
    return std::string((char*)data.data() + 4, data.size() - 4);
}

uint32_t PssgAttribute::uint32() const {
    if (data.size() != 4) {
        THROW_OR_ABORT("PSSG uint32 attribute does not have 4 bytes");
    }
    return swap_endianness(*reinterpret_cast<const uint32_t*>(data.data()));
}

uint64_t PssgAttribute::uint64() const {
    if (data.size() != 8) {
        THROW_OR_ABORT("PSSG uint64 attribute does not have 8 bytes");
    }
    return swap_endianness(*reinterpret_cast<const uint64_t*>(data.data()));
}

float PssgAttribute::float32() const {
    if (data.size() != 4) {
        THROW_OR_ABORT("PSSG float attribute does not have 4 bytes");
    }
    return swap_endianness(*reinterpret_cast<const float*>(data.data()));
}

FixedArray<double, 2> PssgAttribute::dvec2() const {
    if (data.size() != 16) {
        THROW_OR_ABORT("PSSG dvec2 attribute does not have 16 bytes");
    }
    FixedArray<double, 2> result = uninitialized;
    result(0) = swap_endianness(*reinterpret_cast<const double*>(data.data()));
    result(1) = swap_endianness(*reinterpret_cast<const double*>(data.data() + 8));
    return result;
}

const PssgNode& PssgNode::get_child(
    const std::string& type,
    const PssgSchema& schema) const
{
    const PssgNode* result = nullptr;
    for (const auto& c : children) {
        if (schema.nodes.get(c.type_id).name == type) {
            if (result != nullptr) {
                THROW_OR_ABORT("Found multiple children of type \"" + type + '"');
            }
            result = &c;
        }
    }
    if (result == nullptr) {
        THROW_OR_ABORT("Could not find child of type \"" + type + '"');
    }
    return *result;
}

uint32_t PssgNode::nchildren(const std::string& type, const PssgSchema& schema) const {
    uint32_t result = 0;
    for (const auto& c : children) {
        if (schema.nodes.get(c.type_id).name == type) {
            ++result;
        }
    }
    return result;
}

const PssgAttribute& PssgNode::get_attribute(
    const std::string& name,
    const PssgSchema& schema) const
{
    for (const auto& [i, a] : attributes) {
        if (schema.attributes.get(i).name == name) {
            return a;
        }
    }
    THROW_OR_ABORT("Could not find attribute with name \"" + name + '"');
}

bool PssgNode::has_attribute(const std::string& name, const PssgSchema& schema) const {
    for (const auto& [i, a] : attributes) {
        if (schema.attributes.get(i).name == name) {
            return true;
        }
    }
    return false;
}

bool PssgNode::for_each_node(const std::function<bool(const PssgNode& node)>& op) const
{
    if (!op(*this)) {
        return false;
    }
    for (const auto& c : children) {
        if (!c.for_each_node(op)) {
            return false;
        }
    }
    return true;
}

std::string PssgNode::pnstring() const {
    auto str = std::string((const char*)data.data(), data.size());
    if (str.empty()) {
        THROW_OR_ABORT("Raw PNSTRING is empty");
    }
    if (str[str.size() - 1] != 0) {
        THROW_OR_ABORT("PNSTRING is not null-terminated");
    }
    return str.substr(0, str.size() - 1);
}

template <class TData>
TData PssgNode::scalar() const {
    if (data.size() != sizeof(TData)) {
        THROW_OR_ABORT("PSSG scalar attribute does not have correct number of bytes");
    }
    const TData* src = reinterpret_cast<const TData*>(data.data());
    return swap_endianness(*src);
}

template <class TData, size_t... tshape>
FixedArray<TData, tshape...> PssgNode::array() const {
    if (data.size() != sizeof(FixedArray<TData, tshape...>)) {
        THROW_OR_ABORT("PSSG array attribute does not have correct number of bytes");
    }
    FixedArray<TData, tshape...> result = uninitialized;
    const TData* src = reinterpret_cast<const TData*>(data.data());
    TData* dst = result.flat_begin();
    for (size_t i = 0; i < result.nelements(); ++i) {
        dst[i] = swap_endianness(src[i]);
    }
    return result;
}

AxisAlignedBoundingBox<float, 3> PssgNode::saabb3() const {
    if (data.size() != 24) {
        THROW_OR_ABORT("PSSG saabb3 attribute does not have 24 bytes");
    }
    AxisAlignedBoundingBox<float, 3> result = uninitialized;
    const float* src = reinterpret_cast<const float*>(data.data());
    float* dst = reinterpret_cast<float*>(&result);
    for (size_t i = 0; i < 6; ++i) {
        dst[i] = swap_endianness(src[i]);
    }
    return result;
}

uint32_t fourcc(std::string s) {
    if (s.length() != 4) {
        THROW_OR_ABORT("FOURCC string does not have length 4");
    }
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return *reinterpret_cast<uint32_t*>(s.data());
}

std::vector<std::byte> PssgNode::texture(const PssgSchema& schema) const {
    const bool cube_preview = false;
    DdsHeader header;
    DdsHeaderDxt10 header10;
    std::memset(&header, 0, sizeof(header));
    std::memset(&header10, 0, sizeof(header10));

    header.size = 124;
    header.flags |= DdsFlags::CAPS | DdsFlags::HEIGHT | DdsFlags::WIDTH | DdsFlags::PIXELFORMAT;
    header.ddspf.size = 32;
    header.caps |= DdsCaps::TEXTURE;
    header10.resourceDimension = D3d10ResourceDimension::TEXTURE2D;
    header10.arraySize = 1;

    header.width = get_attribute("width", schema).uint32();
    if (header.width > 10'000) {
        THROW_OR_ABORT("Width too large");
    }
    header.height = get_attribute("height", schema).uint32();
    if (header.height > 10'000) {
        THROW_OR_ABORT("Height too large");
    }
    auto texel_format = get_attribute("texelFormat", schema).string();
    if (texel_format == "dxt1") {
        header.flags |= DdsFlags::LINEARSIZE;
        header.pitchOrLinearSize = (header.height * header.width) / 2;
        header.ddspf.flags |= DdsPixelFormatFlags::FOURCC;
        header.ddspf.fourCC = fourcc(texel_format);
    } else if (texel_format == "dxt1_srgb") {
        header.flags |= DdsFlags::LINEARSIZE;
        header.pitchOrLinearSize = (header.height * header.width) / 2;
        header.ddspf.flags |= DdsPixelFormatFlags::FOURCC;
        header.ddspf.fourCC = fourcc("DX10");
        header10.dxgiFormat = DxgiFormat::BC1_UNORM_SRGB;
    } else if (
        (texel_format == "dxt2") ||
        (texel_format == "dxt3") ||
        (texel_format == "dxt4") ||
        (texel_format == "dxt5"))
    {
        header.flags |= DdsFlags::LINEARSIZE;
        header.pitchOrLinearSize = (header.height * header.width);
        header.ddspf.flags |= DdsPixelFormatFlags::FOURCC;
        header.ddspf.fourCC = fourcc(texel_format);
    } else if (texel_format == "dxt3_srgb") {
        header.flags |= DdsFlags::LINEARSIZE;
        header.pitchOrLinearSize = (header.height * header.width);
        header.ddspf.flags |= DdsPixelFormatFlags::FOURCC;
        header.ddspf.fourCC = fourcc("DX10");
        header10.dxgiFormat = DxgiFormat::BC2_UNORM_SRGB;
    } else if (texel_format == "dxt5_srgb") {
        header.flags |= DdsFlags::LINEARSIZE;
        header.pitchOrLinearSize = (header.height * header.width);
        header.ddspf.flags |= DdsPixelFormatFlags::FOURCC;
        header.ddspf.fourCC = fourcc("DX10");
        header10.dxgiFormat = DxgiFormat::BC3_UNORM_SRGB;
    } else if (texel_format == "bc6h_uf") {
        header.flags |= DdsFlags::LINEARSIZE;
        header.pitchOrLinearSize = (header.height * header.width);
        header.ddspf.flags |= DdsPixelFormatFlags::FOURCC;
        header.ddspf.fourCC = fourcc("DX10");
        header10.dxgiFormat = DxgiFormat::BC6H_UF16;
    } else if (texel_format == "BC7") {
        header.flags |= DdsFlags::LINEARSIZE;
        header.pitchOrLinearSize = (header.height * header.width);
        header.ddspf.flags |= DdsPixelFormatFlags::FOURCC;
        header.ddspf.fourCC = fourcc("DX10");
        header10.dxgiFormat = DxgiFormat::BC7_UNORM;
    } else if (texel_format == "BC7_srgb") {
        header.flags |= DdsFlags::LINEARSIZE;
        header.pitchOrLinearSize = (header.height * header.width);
        header.ddspf.flags |= DdsPixelFormatFlags::FOURCC;
        header.ddspf.fourCC = fourcc("DX10");
        header10.dxgiFormat = DxgiFormat::BC7_UNORM_SRGB;
    } else if (
        (texel_format == "ui8x4") ||
        (texel_format == "u8x4"))
    {
        header.flags |= DdsFlags::LINEARSIZE;
        header.pitchOrLinearSize = (header.height * header.width) * 4;
        header.ddspf.flags |= DdsPixelFormatFlags::ALPHAPIXELS | DdsPixelFormatFlags::RGB;
        header.ddspf.fourCC = 0;
        header.ddspf.RGBBitCount = 32;
        header.ddspf.RBitMask = 0xFF0000;
        header.ddspf.GBitMask = 0xFF00;
        header.ddspf.BBitMask = 0xFF;
        header.ddspf.ABitMask = 0xFF000000;
    } else if (texel_format == "u8") {
        header.flags |= DdsFlags::LINEARSIZE;
        header.pitchOrLinearSize = (header.height * header.width);
        header.ddspf.flags |= DdsPixelFormatFlags::LUMINANCE;
        header.ddspf.fourCC = 0;
        header.ddspf.RGBBitCount = 8;
        header.ddspf.RBitMask = 0xFF;
    } else {
        THROW_OR_ABORT("Texel format not supported: \"" + texel_format + '"');
    }
    // Mip Maps
    header.mipMapCount = 1;

    if (has_attribute("automipmap", schema) &&
        has_attribute("numberMipMapLevels", schema))
    {
        if ((get_attribute("automipmap", schema).uint32() == 0) &&
            (get_attribute("numberMipMapLevels", schema).uint32() > 0))
        {
            header.flags |= DdsFlags::MIPMAPCOUNT;
            header.mipMapCount = get_attribute("numberMipMapLevels", schema).uint32() + 1;
            header.caps |= DdsCaps::MIPMAP | DdsCaps::COMPLEX;
        }
    }
    std::vector<std::byte> bdata;
    std::map<uint32_t, std::vector<std::byte>> bdata2;
    // Byte Data
    if (has_attribute("imageBlockCount", schema)) {
        const auto& texture_image_blocks = get_child("TEXTUREIMAGEBLOCK", schema);
        if (get_attribute("imageBlockCount", schema).uint32() > 1) {
            texture_image_blocks.for_each_node([&](const auto& texture_image_block) {
                auto typename_ = texture_image_block.get_attribute("typename", schema).string();
                if (typename_ == "Raw") {
                    header.caps2 |= DdsCaps2::CUBEMAP_POSITIVEX;
                    bdata2[0] = texture_image_block.get_child("TEXTUREIMAGEBLOCKDATA", schema).data;
                    return true;
                }
                if (typename_ == "RawNegativeX") {
                    header.caps2 |= DdsCaps2::CUBEMAP_NEGATIVEX;
                    bdata2[1] = texture_image_block.get_child("TEXTUREIMAGEBLOCKDATA", schema).data;
                    return true;
                }
                if (typename_ == "RawPositiveY") {
                    header.caps2 |= DdsCaps2::CUBEMAP_POSITIVEY;
                    bdata2[2] = texture_image_block.get_child("TEXTUREIMAGEBLOCKDATA", schema).data;
                    return true;
                }
                if (typename_ == "RawNegativeY") {
                    header.caps2 |= DdsCaps2::CUBEMAP_NEGATIVEY;
                    bdata2[3] = texture_image_block.get_child("TEXTUREIMAGEBLOCKDATA", schema).data;
                    return true;
                }
                if (typename_ == "RawPositiveZ") {
                    header.caps2 |= DdsCaps2::CUBEMAP_POSITIVEZ;
                    bdata2[4] = texture_image_block.get_child("TEXTUREIMAGEBLOCKDATA", schema).data;
                    return true;
                }
                if (typename_ == "RawNegativeZ") {
                    header.caps2 |= DdsCaps2::CUBEMAP_NEGATIVEZ;
                    bdata2[5] = texture_image_block.get_child("TEXTUREIMAGEBLOCKDATA", schema).data;
                    return true;
                }
                return true;
            });
            if (cube_preview) {
                header.caps2 = DdsCaps2::NONE;
            } else if (bdata2.size() == get_attribute("imageBlockCount", schema).uint32()) {
                header.caps2 |= DdsCaps2::CUBEMAP;
                header.flags = header.flags ^ DdsFlags::LINEARSIZE;
                header.pitchOrLinearSize = 0;
                header.caps |= DdsCaps::COMPLEX;
            } else {
                THROW_OR_ABORT("Loading cubemap failed because not all blocks were found. (Read)");
            }
        } else {
            bdata = texture_image_blocks.get_child("TEXTUREIMAGEBLOCKDATA", schema).data;
        }
    } else {
        auto nc = nchildren("TEXTUREIMAGE", schema);
        if (nc == 1) {
            bdata = get_child("TEXTUREIMAGE", schema).data;
        } else {
            THROW_OR_ABORT("Support for exporting this texture is not implemented.");
        }
    }

    std::ostringstream sstr{ std::ios::binary };
    write_binary(sstr, DDS_MAGIC, "magic");
    write_binary(sstr, header, "header");
    if (any(header.ddspf.flags & DdsPixelFormatFlags::FOURCC) &&
        (header.ddspf.fourCC == 808540228)) // DX10
    {
        write_binary(sstr, header10, "header10");
    }
    if (!bdata2.empty()) {
        for (const auto& [_, b2] : bdata2) {
            write_iterable(sstr, b2, "cubemap block");
        }
    } else {
        write_iterable(sstr, bdata, "texture");
    }
    auto res = sstr.str();
    return { (const std::byte*)res.data(), (const std::byte*)res.data() + res.size() };
}

namespace Mlib {

template float PssgNode::scalar() const;
template FixedArray<float, 2> PssgNode::array() const;
template FixedArray<float, 3> PssgNode::array() const;
template FixedArray<float, 4> PssgNode::array() const;
template FixedArray<float, 4, 4> PssgNode::array() const;

}
