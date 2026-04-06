#include "Compressed_File.hpp"
#include <Mlib/Compression/Decompress.hpp>
#include <Mlib/Os/Os.hpp>
#include <sstream>

using namespace Mlib;

CompressedFile::CompressedFile(std::filesystem::path p)
    : path_{std::move(p)}
{}

CompressedFile CompressedFile::sibling(
    const std::filesystem::path& filename) const
{
    if (path_.extension() == ".gz") {
        return CompressedFile{path_.parent_path() / (filename.string() + ".gz")};
    } else {
        return CompressedFile{path_.parent_path() / filename};
    }
}

std::filesystem::path CompressedFile::path() const {
    return path_;
}

bool CompressedFile::has_any_extension(std::initializer_list<std::string> candidates) const {
    auto s = path_.string();
    for (const auto& c : candidates) {
        if (s.ends_with(c) || s.ends_with(c + ".gz")) {
            return true;
        }
    }
    return false;
}

std::unique_ptr<std::istream> CompressedFile::decompressed_ifstream() const {
    if (path_.extension() == ".gz") {
        auto compressed = create_ifstream(path_);
        if (compressed->fail()) {
            throw std::runtime_error("Could not open for read: \"" + path_.string() + '"');
        }
        auto begin = compressed->tellg();
        compressed->seekg(0, std::ios::end);
        auto end = compressed->tellg();
        compressed->seekg(0);
        return std::make_unique<std::istringstream>(uncompress_stream(*compressed, path_, end - begin));
    } else {
        return create_ifstream(path_);
    }
}
