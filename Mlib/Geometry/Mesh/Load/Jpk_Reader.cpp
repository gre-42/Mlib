#include "Jpk_Reader.hpp"
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Io/Endian.hpp>
#include <Mlib/Io/Stream_And_Lock.hpp>
#include <Mlib/Io/Stream_Segment.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <istream>

using namespace Mlib;

struct JpkHeader {
    uint32_t magic;
    uint32_t unknown0;
    uint32_t nentries;
    uint32_t alignment;
    uint32_t unknown1;
    uint32_t offset_to_filenames;
    uint32_t unknown2;
    uint32_t unknown3;
};
static_assert(sizeof(JpkHeader) == 32);

JpkReader::JpkReader(std::unique_ptr<std::istream>&& data, IoVerbosity verbosity)
    : directory_{ "JPK entry" }
    , reading_{ false }
{
    auto header = read_binary<JpkHeader>(*data, "JPK header", verbosity);
    if (header.magic != 0x4B41504A) {
        THROW_OR_ABORT("Wrong JPK header");
    }
    if (any(verbosity & IoVerbosity::METADATA)) {
        linfo() << "#Entries: " << header.nentries;
        linfo() << "Alignment: " << header.alignment;
        linfo() << "Offset to filenames: " << header.offset_to_filenames;
    }
    for (size_t i = 0; i < header.nentries; ++i) {
        if (any(verbosity & IoVerbosity::METADATA)) {
            linfo() << "Entry index: " << i;
        }
        data->seekg(integral_cast<std::streamoff>(32 + i * 32));
        
        auto name_offset = read_binary<uint32_t>(*data, "name offset", verbosity);
        auto data_size = read_binary<uint32_t>(*data, "data size", verbosity);
        auto file_offset = read_binary<uint32_t>(*data, "file offset", verbosity);

        if (any(verbosity & IoVerbosity::METADATA)) {
            linfo() << "Name offset: " << name_offset;
            linfo() << "Data size: " << data_size;
            linfo() << "File offset: " << file_offset;
        }
        data->seekg(name_offset);
        while (read_binary<char>(*data, "name character", verbosity) != 0);
        auto end = data->tellg();
        data->seekg(name_offset);
        auto name_length = end - integral_cast<std::streamoff>(name_offset);
        if (name_length == 0) {
            THROW_OR_ABORT("Raw null-terminated string is empty");
        }
        if (name_length > 1000) {
            THROW_OR_ABORT("Name too long");
        }
        auto name = VariableAndHash<std::string>{read_string(
            *data,
            integral_cast<size_t>((std::streamoff)name_length - integral_cast<std::streamoff>(1)),
            "name",
            verbosity)};
        data->seekg(file_offset);
        directory_.add(name, file_offset, data_size);
        if (any(verbosity & IoVerbosity::METADATA)) {
            linfo() << "Name: " << *name;
        }
    }
    data_ = std::move(data);
}

std::shared_ptr<IIStreamDictionary> JpkReader::load_from_file(
    const std::string& filename,
    IoVerbosity verbosity)
{
    auto f = create_ifstream(filename, std::ios::binary);
    if (f->fail()) {
        THROW_OR_ABORT("Could not open \"" + filename + '"');
    }
    return std::make_shared<JpkReader>(std::move(f), verbosity);
}

JpkReader::~JpkReader() = default;

std::vector<VariableAndHash<std::string>> JpkReader::names() const {
    return directory_.keys();
}

StreamAndSize JpkReader::read(
    const VariableAndHash<std::string>& name,
    std::ios::openmode openmode,
    SourceLocation loc)
{
    if (openmode != std::ios::binary) {
        THROW_OR_ABORT("Open-mode is not binary");
    }
    const auto& v = directory_.get(name);
    auto lock = std::make_unique<std::scoped_lock<std::recursive_mutex>>(mutex_);
    if (reading_) {
        THROW_OR_ABORT("Recursively reading from JPK is not supported");
    }
    reading_ = true;
    data_->seekg(v.offset);
    if (data_->fail()) {
        THROW_OR_ABORT("Could not seek entry \"" + *name + '"');
    }
    auto stream = std::make_unique<IStreamAndLock<DanglingBaseClassRef<JpkReader>>>(
        *data_,
        std::move(lock),
        DanglingBaseClassRef<JpkReader>{ *this, loc });
    return { std::move(stream), v.size };
}
