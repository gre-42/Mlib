#pragma once
#include <Mlib/Io/IIStream_Dictionary.hpp>
#include <Mlib/Map/Map.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <memory>
#include <mutex>
#include <string>

namespace Mlib {

enum class IoVerbosity;
struct StreamSegment;
template <class TStreamOwner>
class IStreamAndLock;

class JpkReader: public IIStreamDictionary, public virtual DanglingBaseClass {
    JpkReader(const JpkReader&) = delete;
    JpkReader& operator = (const JpkReader& other) = delete;
    friend IStreamAndLock<DanglingBaseClassRef<JpkReader>>;
public:
    JpkReader(std::unique_ptr<std::istream>&& data, IoVerbosity verbosity);
    static std::shared_ptr<IIStreamDictionary> load_from_file(
        const std::string& filename,
        IoVerbosity verbosity);
    virtual ~JpkReader() override;
    virtual std::vector<std::string> names() const override;
    virtual StreamAndSize read(
        const std::string& name,
        std::ios::openmode openmode,
        SourceLocation loc) override;
private:
    Map<std::string, StreamSegment> directory_;
    std::unique_ptr<std::istream> data_;
    std::recursive_mutex mutex_;
    bool reading_;
};

}
