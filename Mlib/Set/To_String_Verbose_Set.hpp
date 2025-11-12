#pragma once
#include <Mlib/Set/Verbose_Set.hpp>
#include <Mlib/Variable_And_Hash.hpp>

namespace Mlib {

template <class TBaseMap>
class ToStringVerboseSet: public VerboseSet<TBaseMap> {
public:
    using value_type = TBaseMap::value_type;
    using key_type = TBaseMap::key_type;
    using node_type = TBaseMap::node_type;
    using iterator = TBaseMap::iterator;
    using const_iterator = TBaseMap::const_iterator;

    explicit ToStringVerboseSet(std::string value_name)
        : VerboseSet<TBaseMap>{
            std::move(value_name),
            [](const key_type& key){ return std::to_string(key); } }
    {}
    ~ToStringVerboseSet() = default;
};

}
