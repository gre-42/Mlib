#pragma once
#include <Mlib/Io/IIStream_Dictionary.hpp>

namespace Mlib {

class FolderIStreamDictionary: public IIStreamDictionary {
public:
    explicit FolderIStreamDictionary(std::string folder);
    virtual ~FolderIStreamDictionary() override;
    virtual std::vector<VariableAndHash<std::string>> names() const override;
    virtual StreamAndSize read(
        const VariableAndHash<std::string>& name,
        std::ios::openmode openmode,
        SourceLocation loc) override;
private:
    std::string folder_;
};

}
