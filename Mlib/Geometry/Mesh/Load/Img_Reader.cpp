#include "Img_Reader.hpp"
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Io/Cleanup.hpp>
#include <Mlib/Io/Stream_And_Lock.hpp>
#include <Mlib/Io/Stream_Segment.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <istream>

using namespace Mlib;

struct DirectoryInfo {
    uint32_t offset;
    uint32_t size;
    char name[24];
};
static_assert(sizeof(DirectoryInfo) == 32);

ImgReader::ImgReader(std::istream& directory, std::unique_ptr<std::istream>&& data)
    : directory_{ "IMG entry" }
    , reading_{ false }
{
    while (directory.peek() != EOF) {
        auto h = read_binary<DirectoryInfo>(directory, "directory entry", IoVerbosity::SILENT);
        auto entry_name = VariableAndHash<std::string>{remove_trailing_zeros(std::string(h.name, sizeof(h.name)))};
        // linfo() << "Entry name: " << entry_name;
        directory_.add(
            entry_name,
            std::streamoff{ h.offset } << 11,
            integral_cast<std::streamsize>(h.size) << 11);
    }
    data_ = std::move(data);
}

std::shared_ptr<IIStreamDictionary> ImgReader::load_from_file(const std::string& img_filename) {
    if (!img_filename.ends_with(".img")) {
        THROW_OR_ABORT("Filename \"" + img_filename + "\" does not end with .img");
    }
    auto dir_filename = img_filename.substr(0, img_filename.length() - 3) + "dir";

    auto dir = create_ifstream(dir_filename, std::ios::binary);
    if (dir->fail()) {
        THROW_OR_ABORT("Could not open \"" + dir_filename + '"');
    }

    auto img = create_ifstream(img_filename, std::ios::binary);
    if (img->fail()) {
        THROW_OR_ABORT("Could not open \"" + img_filename + '"');
    }
    return std::make_shared<ImgReader>(*dir, std::move(img));
}

ImgReader::~ImgReader() = default;

std::vector<VariableAndHash<std::string>> ImgReader::names() const {
    return directory_.keys();
}

StreamAndSize ImgReader::read(
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
        THROW_OR_ABORT("Recursively reading from IMG is not supported");
    }
    reading_ = true;
    data_->seekg(v.offset);
    if (data_->fail()) {
        THROW_OR_ABORT("Could not seek entry \"" + *name + '"');
    }
    auto stream = std::make_unique<IStreamAndLock<DanglingBaseClassRef<ImgReader>>>(
        *data_,
        std::move(lock),
        DanglingBaseClassRef<ImgReader>{ *this, loc });
    return { std::move(stream), v.size };
}
