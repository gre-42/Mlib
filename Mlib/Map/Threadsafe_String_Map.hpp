#pragma once
#include <Mlib/Map/Threadsafe_String_Generic_Map.hpp>
#include <map>

namespace Mlib {

template <class TValue>
using ThreadsafeStringMap = ThreadsafeStringGenericMap<std::map<std::string, TValue>>;

}
