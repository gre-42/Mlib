#pragma once
#include <filesystem>
#include <iosfwd>
#include <memory>

namespace Mlib {

class CompressedFile {
public:
    explicit CompressedFile(std::filesystem::path p);
    CompressedFile sibling(const std::filesystem::path& filename) const;
    std::filesystem::path path() const;
    bool has_any_extension(std::initializer_list<std::string> candidates) const;
    template <class... TArgs>
    bool has_any_extension(TArgs&&... args) const {
        return has_any_extension({args...});
    }
    std::unique_ptr<std::istream> decompressed_ifstream() const;
private:
    std::filesystem::path path_;
};

}
