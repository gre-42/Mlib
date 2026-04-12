#pragma once
#include <Mlib/Strings/Utf8_Path.hpp>
#include <iosfwd>
#include <memory>

namespace Mlib {

class CompressedFile {
public:
    explicit CompressedFile(Utf8Path p);
    CompressedFile sibling(const Utf8Path& filename) const;
    Utf8Path path() const;
    bool has_any_extension(std::initializer_list<std::string> candidates) const;
    template <class... TArgs>
    bool has_any_extension(TArgs&&... args) const {
        return has_any_extension({args...});
    }
    std::unique_ptr<std::istream> decompressed_ifstream() const;
private:
    Utf8Path path_;
};

}
