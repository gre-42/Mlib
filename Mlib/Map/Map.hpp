#include <Mlib/Map/Generic_Map.hpp>
#include <map>

namespace Mlib {

template <class TKey, class TValue>
using Map = GenericMap<std::map<TKey, TValue>>;

}
