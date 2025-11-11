#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Set/String_With_Hash_Unordered_Set.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <string>

namespace Mlib {

struct CreatedAtRemoteSite: public DanglingBaseClass {
    CreatedAtRemoteSite();
    ~CreatedAtRemoteSite();
    StringWithHashUnorderedSet rigid_bodies;
    StringWithHashUnorderedSet players;
};

}
