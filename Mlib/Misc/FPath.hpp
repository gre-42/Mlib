#pragma once
#include <Mlib/Hashing/Std_Hash.hpp>
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Strings/Utf8_Path.hpp>
#include <compare>
#include <iosfwd>
#include <list>
#include <string>
#include <string_view>

namespace Mlib {

enum class PathType {
    EMPTY,
    LOCAL_PATH,
    VARIABLE
};

class FPath {
public:
    FPath();
    explicit FPath(const std::u8string_view& uri);
    ~FPath();
    static FPath from_local_path(const Utf8Path& path);
    static FPath from_variable(std::string name);
    static FPath from_variable_and_hash(VariableAndHash<std::string> name);
    PathType type() const;
    bool empty() const;
    Utf8Path local_path() const;
    std::string variable() const;
    std::string string() const;
    VariableAndHash<std::string> variable_and_hash() const;
    VariableAndHash<std::string> string_and_hash() const;
    size_t hash() const;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(type_);
        archive(path_or_variable_);
    }
    std::strong_ordering operator <=> (const FPath&) const = default;
private:
    FPath(PathType type, const std::string_view& path_or_variable);
    FPath(PathType type, std::string path_or_variable);
    FPath(PathType type, VariableAndHash<std::string> path_or_variable);
    PathType type_;
    VariableAndHash<std::string> path_or_variable_;
};

std::ostream& operator << (std::ostream& ostr, const FPath& fpath);

}

template <>
struct std::hash<Mlib::FPath>
{
    inline std::size_t operator() (const Mlib::FPath& k) const {
        return k.hash();
    }
};
