#pragma once
#include <Mlib/Io/IIStream_Dictionary.hpp>
#include <Mlib/Map/Map.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <memory>
#include <mutex>
#include <string>

namespace Mlib {

struct EntryInfo {
	std::streamoff offset;
	std::streamsize size;
};

class ImgReader: public IIStreamDictionary, public DanglingBaseClass {
	ImgReader(const ImgReader&) = delete;
	ImgReader& operator = (ImgReader&& other) = delete;
public:
	ImgReader(std::istream& directory, std::unique_ptr<std::istream>&& data);
	static std::shared_ptr<IIStreamDictionary> load_from_file(const std::string& img_filename);
	virtual ~ImgReader() override;
	virtual std::vector<std::string> names() const override;
	virtual std::unique_ptr<std::istream> read(
		const std::string& name,
		std::ios::openmode openmode,
		SourceLocation loc) override;
private:
	Map<std::string, EntryInfo> directory_;
	std::unique_ptr<std::istream> data_;
	std::mutex mutex_;
};

}
