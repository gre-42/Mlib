#include <Mlib/Map/Generic_Map.hpp>
#include <unordered_map>

namespace Mlib {

template <class TKey, class TValue>
using UnorderedMap = GenericMap<std::unordered_map<TKey, TValue>>;

}
