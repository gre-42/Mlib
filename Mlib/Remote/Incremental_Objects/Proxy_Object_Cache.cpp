#include "Proxy_Object_Cache.hpp"
#include <Mlib/Hashing/Hash.hpp>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

bool ChannelPair::operator == (const ChannelPair& other) const = default;

std::size_t ChannelPairHash::operator () (const ChannelPair& pair) const {
    return hash_combine(pair.proxy_id, pair.object_id);
}

void ProxyObjectsCaches::add(
    RemoteSiteId proxy_id,
    RemoteObjectId object_id,
    std::unique_ptr<ProxyObjectCacheEntry>&& cache)
{
    ChannelPair pair{proxy_id, object_id};
    
    if (!cache_registry_.try_emplace(pair, std::move(cache)).second) {
        throw std::runtime_error("Communication channel already registered");
    }
    proxy_index_.emplace(proxy_id, pair);
    object_index_.emplace(object_id, pair);
}

ProxyObjectCacheEntry* ProxyObjectsCaches::try_get(RemoteSiteId proxy_id, RemoteObjectId object_id) {
    auto it = cache_registry_.find({proxy_id, object_id});
    return (it != cache_registry_.end()) ? it->second.get() : nullptr;
}

void ProxyObjectsCaches::remove_proxy(RemoteSiteId proxy_id) {
    auto range = proxy_index_.equal_range(proxy_id);
    
    for (auto it = range.first; it != range.second; ++it) {
        ChannelPair pair = it->second;
        
        cache_registry_.erase(pair);
        
        auto obj_range = object_index_.equal_range(pair.object_id);
        for (auto o_it = obj_range.first; o_it != obj_range.second; ++o_it) {
            if (o_it->second.proxy_id == proxy_id) {
                object_index_.erase(o_it);
                break;
            }
        }
    }
    proxy_index_.erase(proxy_id);
}

void ProxyObjectsCaches::remove_object(RemoteObjectId object_id) {
    auto range = object_index_.equal_range(object_id);
    for (auto it = range.first; it != range.second; ++it) {
        ChannelPair pair = it->second;
        cache_registry_.erase(pair);

        auto proxy_range = proxy_index_.equal_range(pair.proxy_id);
        for (auto p_it = proxy_range.first; p_it != proxy_range.second; ++p_it) {
            if (p_it->second.object_id == object_id) {
                proxy_index_.erase(p_it);
                break;
            }
        }
    }
    object_index_.erase(object_id);
}

void ProxyObjectsCaches::print_status(std::ostream& ostr) const {
    ostr <<
        "Registry size: " << cache_registry_.size() <<
        " | proxy indices: " << proxy_index_.size() <<
        " | object indices: " << object_index_.size();
}
