
#include "FPath.hpp"
#include <Mlib/Hashing/Hash.hpp>
#include <Mlib/Os/Os.hpp>
#include <sstream>
#include <stdexcept>
#include <string_view>

using namespace Mlib;
using namespace std::literals;

static const auto LOCAL_PATH_PREFIX = "file://"sv;
static const auto VARIABLE_PREFIX = "#"sv;

FPath::FPath()
    : type_{PathType::EMPTY}
{}

FPath::FPath(const std::string_view& uri) {
    if (uri.starts_with(LOCAL_PATH_PREFIX)) {
        type_ = PathType::LOCAL_PATH;
        path_or_variable_ = VariableAndHash{(std::string)uri.substr(LOCAL_PATH_PREFIX.length())};
        if (path_or_variable_->empty()) {
            throw std::runtime_error("Path is empty");
        }
        return;
    }
    if (uri.starts_with(VARIABLE_PREFIX)) {
        type_ = PathType::VARIABLE;
        path_or_variable_ = VariableAndHash{(std::string)uri.substr(VARIABLE_PREFIX.length())};
        if (path_or_variable_->empty()) {
            throw std::runtime_error("Variable name is empty");
        }
        return;
    }
    throw std::runtime_error("Unkown URI type: \"" + std::string{uri} + '"');
}

FPath::FPath(PathType type, const std::string_view& path_or_variable)
    : type_{type}
    , path_or_variable_{(std::string)path_or_variable}
{}

FPath::FPath(PathType type, std::string path_or_variable)
    : type_{type}
    , path_or_variable_{std::move(path_or_variable)}
{}

FPath::FPath(PathType type, VariableAndHash<std::string> path_or_variable)
    : type_{type}
    , path_or_variable_{std::move(path_or_variable)}
{}

FPath::~FPath() = default;

FPath FPath::from_local_path(const std::filesystem::path& path) {
    if (path.empty()) {
        throw std::runtime_error("FPath local path is empty");
    }
    return { PathType::LOCAL_PATH, path.string() };
}

FPath FPath::from_variable(std::string name) {
    if (name.empty()) {
        throw std::runtime_error("FPath variable name is empty");
    }
    return { PathType::VARIABLE, std::move(name) };
}

FPath FPath::from_variable_and_hash(VariableAndHash<std::string> name) {
    if (name->empty()) {
        throw std::runtime_error("FPath variable name is empty");
    }
    return { PathType::VARIABLE, std::move(name) };
}

PathType FPath::type() const {
    return type_;
}

bool FPath::empty() const {
    return type_ == PathType::EMPTY;
}

std::filesystem::path FPath::local_path() const {
    if (type_ != PathType::LOCAL_PATH) {
        throw std::runtime_error("URI is not a local path: \"" + *path_or_variable_ + '"');
    }
    return *path_or_variable_;
}

std::string FPath::variable() const {
    if (type_ != PathType::VARIABLE) {
        throw std::runtime_error("URI is not a variable: \"" + *path_or_variable_ + '"');
    }
    return *path_or_variable_;
}

VariableAndHash<std::string> FPath::variable_and_hash() const {
    if (type_ != PathType::VARIABLE) {
        throw std::runtime_error("URI is not a variable: \"" + *path_or_variable_ + '"');
    }
    return path_or_variable_;
}

VariableAndHash<std::string> FPath::string_and_hash() const {
    return path_or_variable_;
}

std::string FPath::string() const {
    return (std::stringstream() << *this).str();
}

size_t FPath::hash() const {
    return hash_combine(type_, path_or_variable_);
}

std::ostream& Mlib::operator << (std::ostream& ostr, const FPath& fpath) {
    switch (fpath.type()) {
    case PathType::EMPTY:
        ostr << "()";
        return ostr;
    case PathType::LOCAL_PATH:
        ostr << "file://" << fpath.local_path().string();
        return ostr;
    case PathType::VARIABLE:
        ostr << "#" << fpath.variable();
        return ostr;
    }
    throw std::runtime_error("Unknown path type: " + std::to_string((int)fpath.type()));
}
