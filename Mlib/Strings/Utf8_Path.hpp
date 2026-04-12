#pragma once
#include <filesystem>
#include <string>

namespace Mlib {

class Utf8Path {
    static const struct ConstructPathType {} construct_path;
    friend std::ostream& operator << (std::ostream& ostr, const Utf8Path& p);
    friend bool operator == (const Utf8Path& a, const Utf8Path& b);
    friend bool operator != (const Utf8Path& a, const Utf8Path& b);
public:
    Utf8Path();
    static Utf8Path from_path(std::filesystem::path p);
    Utf8Path(std::u8string s);
    Utf8Path(std::string s);
    Utf8Path(std::string_view s);
    Utf8Path(const char* s);
    ~Utf8Path();
    // Concatenation
    Utf8Path& operator /= (const Utf8Path& rhs);
    Utf8Path& operator += (const Utf8Path& rhs);
    Utf8Path operator + (const Utf8Path& rhs) const;
    Utf8Path operator / (const Utf8Path& rhs) const;
    // Modifiers
    Utf8Path& replace_extension(const Utf8Path& replacement);
    // Generation
    Utf8Path lexically_relative( const Utf8Path& base ) const;
    Utf8Path lexically_normal() const;
    Utf8Path absolute() const;
    // Decomposition
    Utf8Path relative_path() const;
    Utf8Path parent_path() const;
    Utf8Path filename() const;
    Utf8Path stem() const;
    Utf8Path extension() const;
    bool ends_with(const Utf8Path& suffix) const;
    // Format observers
    operator const std::filesystem::path& () const;
    operator std::string () const;
    std::string string() const;
    std::u8string u8string() const;
    const char* c_str() const;
    // Queries
    bool empty() const;
    bool is_absolute() const;
    bool is_relative() const;
    std::strong_ordering operator <=> (const Utf8Path& rhs) const;
private:
    Utf8Path(std::filesystem::path p, ConstructPathType);
    std::filesystem::path path_;
    mutable std::u8string u8path_;
};

std::ostream& operator << (std::ostream& ostr, const Utf8Path& p);
bool operator == (const Utf8Path& a, const Utf8Path& b);
bool operator != (const Utf8Path& a, const Utf8Path& b);

}
