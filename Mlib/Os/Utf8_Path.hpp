#pragma once
#include <filesystem>
#include <string>

namespace Mlib {

class Utf8Path {
public:
    inline Utf8Path(const std::string& s)
        : path_{std::u8string((char8_t*)s.data(), s.size())}
    {}
    inline Utf8Path(std::filesystem::path p)
        : path_{std::move(p)}
    {}
    inline ~Utf8Path() = default;
    // Concatenation
    Utf8Path& operator /= (const Utf8Path& rhs) {
        path_ /= rhs.path_;
        return *this;
    }
    Utf8Path& operator += (const Utf8Path& rhs) {
        path_ += rhs.path_;
        return *this;
    }
    // Decomposition
    Utf8Path relative_path() const {
        return path_.relative_path();
    }
    Utf8Path parent_path() const {
        return path_.parent_path();
    }
    Utf8Path filename() const {
        return path_.filename();
    }
    Utf8Path stem() const {
        return path_.stem();
    }
    Utf8Path extension() const {
        return path_.extension();
    }
    // Format observers
    inline operator const std::filesystem::path& () const {
        return path_;
    }
    inline operator std::string () const {
        return this->string();
    }
    inline std::string string() const {
        auto res = path_.u8string();
        return std::string((char*)res.data(), res.size());
    }
    // Queries
    inline bool empty() const {
        return path_.empty();
    }
    inline bool is_absolute() const {
        return path_.is_absolute();
    }
    inline bool is_relative() const {
        return path_.is_relative();
    }
    // IO
    std::ostream& operator << (std::ostream& ostr) const {
        return ostr << path_;
    }
private:
    std::filesystem::path path_;
};

}
