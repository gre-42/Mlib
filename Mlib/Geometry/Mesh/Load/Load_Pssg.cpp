#include "Load_Pssg.hpp"
#include <Mlib/Geometry/Mesh/Load/Pssg_Elements.hpp>
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Io/Endian.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <list>
#include <vector>
#ifndef WITHOUT_ZLIB
#include <zlib.h>
#endif

// From: https://github.com/EgoEngineModding/Ego-Engine-Modding/tree/master/src/EgoPssgEditor

using namespace Mlib;

PssgModel Mlib::load_pssg(const std::string& filename, IoVerbosity verbosity) {
    auto f = create_ifstream(filename, std::ios::binary);
    if (f->fail()) {
        THROW_OR_ABORT("Could not open file \"" + filename + '"');
    }
    try {
        return load_pssg(*f, std::numeric_limits<std::streamoff>::max(), verbosity);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("Error reading from file \"" + filename + "\": " + e.what());
    }
}

#ifdef WITHOUT_ZLIB

PssgModel Mlib::load_pssg(std::istream& istr) {
    THROW_OR_ABORT("Loading PSSG files requires zlib");
}

#else

int uncompress2_patched(Bytef *dest, uLongf destLen,
                        const Bytef *source, uLong *sourceLen,
                        const std::function<bool(uLongf chunkLen)>& notifyRead)
{
    z_stream stream;
    int err;
    const uInt max = (uInt)-1;
    uLong len;

    if (destLen == 0) {
        return Z_BUF_ERROR;
    }

    len = *sourceLen;

    stream.next_in = (z_const Bytef *)source;
    stream.avail_in = 0;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    // From: https://stackoverflow.com/questions/1838699/how-can-i-decompress-a-gzip-stream-with-zlib
    err = inflateInit2(&stream, 16 + MAX_WBITS);
    if (err != Z_OK) return err;

    stream.total_out = 0;
    while (true) {
        stream.next_out = dest;
        stream.avail_out = destLen > (uLong)max ? max : (uInt)destLen;

        if (stream.avail_in == 0) {
            stream.avail_in = len > (uLong)max ? max : (uInt)len;
            len -= stream.avail_in;
        }
        auto before = stream.total_out;
        err = inflate(&stream, Z_NO_FLUSH);
        if ((err == Z_OK) || (err == Z_STREAM_END)) {
            if (!notifyRead(stream.total_out - before)) {
                return Z_BUF_ERROR;
            }
        }
        if (err != Z_OK) {
            break;
        }
    }

    *sourceLen -= len + stream.avail_in;

    inflateEnd(&stream);
    return err == Z_STREAM_END ? Z_OK :
           err == Z_NEED_DICT ? Z_DATA_ERROR :
           err == Z_BUF_ERROR ? Z_DATA_ERROR :
           err;
}

std::istringstream uncompress_stream(
    std::istream& istr,
    std::streamoff nbytes,
    size_t chunk_size = 512)
{
    auto begin = istr.tellg();
    std::streampos end;
    if (nbytes == std::numeric_limits<std::streamoff>::max()) {
        istr.seekg(0, std::ios::end);
        end = istr.tellg();
        istr.seekg(begin);
    } else {
        end = begin + nbytes;
    }
    std::vector<char> compressed(end - begin);
    istr.read(compressed.data(), compressed.size());
    if (istr.fail()) {
        THROW_OR_ABORT("Could not compressed data");
    }

    uLongf clength = compressed.size();
    std::string uncompressed_chunk(chunk_size, '?');
    std::list<std::string> uncompressed_chunks;
    auto notifyRead = [&](uLongf chunkSize){
        if (uncompressed_chunks.size() + chunkSize > 1'000'000'000) {
            return false;
        }
        uncompressed_chunks.push_back(uncompressed_chunk.substr(0, chunkSize));
        return true;
    };
    if (int ret = uncompress2_patched(
            (Bytef*)uncompressed_chunk.data(),
            uncompressed_chunk.size(),
            (Bytef*)compressed.data(),
            &clength,
            notifyRead);
        ret != Z_OK)
    {
        istr.seekg(begin);
        switch (ret) {
        case Z_MEM_ERROR:
            THROW_OR_ABORT("Not enough memory for decompression");
        case Z_BUF_ERROR:
            THROW_OR_ABORT("Not enough room in the output buffer");
        case Z_DATA_ERROR:
            THROW_OR_ABORT("Input data corrupted or incomplete");
        }
        verbose_abort("Unknown return code in uncompress: " + std::to_string(ret));
    }
    istr.seekg(begin + integral_cast<std::streamoff>(clength));
    compressed.clear();
    size_t total_size = 0;
    for (const auto& c : uncompressed_chunks) {
        total_size += c.size();
    }
    std::string uncompressed_string;
    uncompressed_string.reserve(total_size);
    for (const auto& c : uncompressed_chunks) {
        uncompressed_string += c;
    }
    return std::istringstream{ uncompressed_string, std::ios::binary | std::ios::in };
}

PssgAttribute load_pssg_attribute(std::istream& istr, IoVerbosity verbosity) {
    PssgAttribute result;
    auto size = swap_endianness(read_binary<uint32_t>(istr, "attribute size", verbosity));
    if (size > 1'000'000'000) {
        THROW_OR_ABORT("Attribute size too large");
    }
    result.data.resize(size);
    read_vector(istr, result.data, "attribute data", verbosity);
    return result;
}

PssgNode load_pssg_node(std::istream& istr, const PssgSchema& schema, IoVerbosity verbosity) {
    PssgNode result;
    auto node_id = swap_endianness(read_binary<uint32_t>(istr, "node ID", verbosity));
    auto node_size = swap_endianness(read_binary<uint32_t>(istr, "node size", verbosity));
    if (node_size > 1'000'000'000) {
        THROW_OR_ABORT("Node size too large");
    }
    auto node_end = istr.tellg() + integral_cast<std::streamoff>(node_size);

    auto attribute_size = swap_endianness(read_binary<uint32_t>(istr, "attribute size", verbosity));
    if (attribute_size > 1'000'000'000) {
        THROW_OR_ABORT("Attribute size too large");
    }
    auto attribute_end = istr.tellg() + integral_cast<std::streamoff>(attribute_size);
    while (istr.tellg() < attribute_end)
    {
        auto attribute_id = swap_endianness(read_binary<uint32_t>(istr, "attribute ID", verbosity));
        result.attributes.add(attribute_id, load_pssg_attribute(istr, verbosity));
    }
    static const std::unordered_map<std::string, bool> IS_DATA_NODE {
        {"BOUNDINGBOX", true},
        {"DATA", true},
        {"DATABLOCKDATA", true},
        {"DATABLOCKBUFFERED", true},
        {"INDEXSOURCEDATA", true},
        {"INVERSEBINDMATRIX", true},
        {"MODIFIERNETWORKINSTANCEUNIQUEMODIFIERINPUT", true},
        {"NeAnimPacketData_B1", true},
        {"NeAnimPacketData_B4", true},
        {"RENDERINTERFACEBOUNDBUFFERED", true},
        {"SHADERINPUT", true},
        {"TEXTUREIMAGEBLOCKDATA", true},
        {"TRANSFORM", true},
        {"PSSGDATABASE", false},
        {"LIBRARY", false},
        {"PNSTRING", false},
        {"SHADERINSTANCE", false},
        {"SHADERGROUP", false},
        {"SHADERINPUTDEFINITION", false},
        {"SEGMENTSET", false},
        {"RENDERDATASOURCE", false},
        {"RENDERINDEXSOURCE", false},
        {"RENDERSTREAM", false},
        {"DATABLOCK", false},
        {"DATABLOCKSTREAM", false}
    };
    const auto& name = schema.nodes.get(node_id).name;
    auto it = IS_DATA_NODE.find(name);
    if (it == IS_DATA_NODE.end()) {
        THROW_OR_ABORT("Could not determine if node is a data node: \"" + name + '"');
    }
    if (it->second) {
        auto size = node_end - istr.tellg();
        if (size > 1'000'000'000) {
            THROW_OR_ABORT("Node size too large (2)");
        }
        result.data.resize(size);
        read_vector(istr, result.data, "node data", verbosity);
    } else {
        while (istr.tellg() < node_end) {
            result.children.push_back(load_pssg_node(istr, schema, verbosity));
        }
    }
    return result;
}

PssgSchema load_pssg_schema(std::istream& istr, IoVerbosity verbosity)
{
    PssgSchema result;

    swap_endianness(read_binary<uint32_t>(istr, "??? attribute info count ???", verbosity));
    auto node_info_count = swap_endianness(read_binary<uint32_t>(istr, "node info count", verbosity));

    for (uint32_t i = 0; i < node_info_count; ++i)
    {
        auto n_id = swap_endianness(read_binary<uint32_t>(istr, "node ID", verbosity));
        auto& node = result.nodes.add(n_id);

        auto node_name_length = swap_endianness(read_binary<uint32_t>(istr, "node name length", verbosity));
        node.name = read_string(istr, node_name_length, "node name", verbosity);

        auto sub_attribute_info_count = swap_endianness(read_binary<uint32_t>(istr, "sub attribute info count", verbosity));
        for (uint32_t j = 0; j < sub_attribute_info_count; j++)
        {
            auto id = swap_endianness(read_binary<uint32_t>(istr, "attribute id", verbosity));
            auto& attribute = node.attributes.add(id);

            auto attribute_name_length = swap_endianness(read_binary<uint32_t>(istr, "attribute name length", verbosity));
            attribute.name = read_string(istr, attribute_name_length, "attribute name", verbosity);
        }
    }

    return result;
}

PssgModel load_uncompressed_pssg(std::istream& istr, IoVerbosity verbosity) {
    auto magic = read_string(istr, 4, "Incorrect magic PPSG string", verbosity);
    read_binary<uint32_t>(istr, "PSSG size", verbosity);
    PssgModel res;
    res.schema = load_pssg_schema(istr, verbosity);
    res.root = load_pssg_node(istr, res.schema, verbosity);
    return res;
}

PssgModel Mlib::load_pssg(std::istream& istr, std::streamoff nbytes, IoVerbosity verbosity) {
    auto str = uncompress_stream(istr, nbytes);
    return load_uncompressed_pssg(str, verbosity);
}

#endif
