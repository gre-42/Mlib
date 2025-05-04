#include "Folder_IStream_Dictionary.hpp"
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <list>

using namespace Mlib;

FolderIStreamDictionary::FolderIStreamDictionary(std::string folder)
    : folder_{ std::move(folder) }
{}

FolderIStreamDictionary::~FolderIStreamDictionary() = default;

std::vector<VariableAndHash<std::string>> FolderIStreamDictionary::names() const {
    std::list<VariableAndHash<std::string>> res;
    for (const auto& it : list_dir(folder_)) {
        res.emplace_back(it.path().string());
    }
    return std::vector(res.begin(), res.end());
}

StreamAndSize FolderIStreamDictionary::read(
    const VariableAndHash<std::string>& name,
    std::ios::openmode openmode,
    SourceLocation loc)
{
    auto f = std::filesystem::path{ folder_ } / *name;
    auto stream = create_ifstream(f, openmode);
    stream->seekg(0, std::ios::end);
    auto size = stream->tellg();
    stream->seekg(0);
    return { std::move(stream), integral_cast<std::streamsize>(size - std::streampos(0)) };
}
