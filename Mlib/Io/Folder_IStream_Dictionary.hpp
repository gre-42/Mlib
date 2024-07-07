#pragma once
#include <Mlib/Io/IIStream_Dictionary.hpp>

namespace Mlib {

class FolderIStreamDictionary: public IIStreamDictionary {
public:
	explicit FolderIStreamDictionary(std::string folder);
	~FolderIStreamDictionary();
	virtual std::vector<std::string> names() const;
	virtual std::unique_ptr<std::istream> read(
		const std::string& name,
		std::ios::openmode openmode,
		SourceLocation loc) override;
private:
	std::string folder_;
};

}
