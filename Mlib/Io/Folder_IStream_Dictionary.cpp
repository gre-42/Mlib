#include "Folder_IStream_Dictionary.hpp"
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

FolderIStreamDictionary::FolderIStreamDictionary(std::string folder)
	: folder_{ std::move(folder) }
{}

FolderIStreamDictionary::~FolderIStreamDictionary() = default;

std::vector<std::string> FolderIStreamDictionary::names() const {
	std::list<std::string> res;
	for (const auto& it : list_dir(folder_)) {
		res.push_back(it.path().string());
	}
	return std::vector(res.begin(), res.end());
}

std::unique_ptr<std::istream> FolderIStreamDictionary::read(
	const std::string& name,
	std::ios::openmode openmode,
	SourceLocation loc)
{
	auto f = std::filesystem::path{ folder_ } / name;
	return create_ifstream(f, openmode);
}
