#include "Utf8_Path.hpp"
#include <Mlib/Strings/Str.hpp>
#include <stdexcept>

using namespace Mlib;

inline bool is_ascii(char c) {
    return (unsigned char)c < 128;
}

Utf8Path::Utf8Path() = default;

Utf8Path Utf8Path::from_path(const std::filesystem::path& p) {
    return {p, construct_path};
}

Utf8Path::Utf8Path(std::u8string s)
    : path_{std::move(s)}
{}

Utf8Path::Utf8Path(std::string s)
    : path_{std::move(s)}
{
    for (char c : s) {
        if (!is_ascii(c)) {
            throw std::runtime_error("Detected non-ascii characters in \"" + s + '"');
        }
    }
}

Utf8Path::Utf8Path(std::string_view s)
    : path_{s}
{
    for (char c : s) {
        if (!is_ascii(c)) {
            throw std::runtime_error("Detected non-ascii characters in \"" + (std::string)s + '"');
        }
    }
}

Utf8Path::Utf8Path(const char* s)
    : path_{s}
{
    for (const char* c = s; *c != 0; ++c) {
        if (!is_ascii(*c)) {
            throw std::runtime_error("Detected non-ascii characters in \"" + std::string{s} + '"');
        }
    }
}

Utf8Path::~Utf8Path() = default;

// Concatenation
Utf8Path& Utf8Path::operator /= (const Utf8Path& rhs) {
    path_ /= rhs.path_;
    return *this;
}

Utf8Path& Utf8Path::operator += (const Utf8Path& rhs) {
    path_ += rhs.path_;
    return *this;
}

Utf8Path Utf8Path::operator + (const Utf8Path& rhs) const {
    return path_.string() + rhs.string();
}

Utf8Path Utf8Path::operator / (const Utf8Path& rhs) const {
    return from_path(path_ / rhs.path_);
}

// Modifiers
Utf8Path& Utf8Path::replace_extension(const Utf8Path& replacement) {
    path_.replace_extension(replacement);
    return *this;
}

// Generation
Utf8Path Utf8Path::lexically_relative( const Utf8Path& base ) const {
    return from_path(path_.lexically_relative(base));
}

Utf8Path Utf8Path::lexically_normal() const {
    return from_path(path_.lexically_normal());
}

Utf8Path Utf8Path::absolute() const {
    return from_path(std::filesystem::absolute(path_));
}

// Decomposition
Utf8Path Utf8Path::relative_path() const {
    return from_path(path_.relative_path());
}

Utf8Path Utf8Path::parent_path() const {
    return from_path(path_.parent_path());
}

Utf8Path Utf8Path::filename() const {
    return from_path(path_.filename());
}

Utf8Path Utf8Path::stem() const {
    return from_path(path_.stem());
}

Utf8Path Utf8Path::extension() const {
    return from_path(path_.extension());
}

bool Utf8Path::ends_with(const Utf8Path& suffix) const {
    return path_.u8string().ends_with(suffix.path_.u8string());
}

// Format observers
Utf8Path::operator const std::filesystem::path& () const {
    return path_;
}

Utf8Path::operator std::string () const {
    return this->string();
}

std::string Utf8Path::string() const {
    return U8::str(path_.u8string());
}

std::u8string Utf8Path::u8string() const {
    return path_.u8string();
}

const char* Utf8Path::c_str() const {
    if constexpr (std::is_same_v<std::filesystem::path::value_type, char>) {
        return path_.c_str();
    } else {
        u8path_ = path_.u8string();
        return U8::str(u8path_.c_str());
    }
}

// Queries
bool Utf8Path::empty() const {
    return path_.empty();
}

bool Utf8Path::is_absolute() const {
    return path_.is_absolute();
}

bool Utf8Path::is_relative() const {
    return path_.is_relative();
}

std::strong_ordering Utf8Path::operator <=> (const Utf8Path& rhs) const = default;

bool Mlib::operator == (const Utf8Path& a, const Utf8Path& b) {
    return a.path_ == b.path_;
}

bool Mlib::operator != (const Utf8Path& a, const Utf8Path& b) {
    return a.path_ != b.path_;
}

Utf8Path::Utf8Path(std::filesystem::path p, ConstructPathType)
    : path_{std::move(p)}
{}

std::ostream& Mlib::operator << (std::ostream& ostr, const Utf8Path& p) {
    return ostr << p.path_;
}
