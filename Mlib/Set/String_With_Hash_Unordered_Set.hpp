#pragma once
#include <Mlib/Set/String_With_Hash_Generic_Set.hpp>
#include <unordered_set>

namespace Mlib {

using StringWithHashUnorderedSet = StringWithHashGenericSet<std::unordered_set<VariableAndHash<std::string>>>;

}
