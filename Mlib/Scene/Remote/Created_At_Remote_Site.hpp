#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <string>
#include <unordered_set>

namespace Mlib {

struct CreatedAtRemoteSite: public DanglingBaseClass {
    std::unordered_set<VariableAndHash<std::string>> rigid_bodies;
};

}
