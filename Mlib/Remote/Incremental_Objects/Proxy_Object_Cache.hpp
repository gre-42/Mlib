#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Misc/Object.hpp>
#include <Mlib/Remote/Incremental_Objects/Remote_Object_Id.hpp>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

namespace Mlib {

using ProxyObjectCacheEntry = Object;

struct ChannelPair {
    RemoteSiteId proxy_id;
    RemoteObjectId object_id;
    bool operator == (const ChannelPair& other) const;
};

struct ChannelPairHash {
    std::size_t operator () (const ChannelPair& pair) const;
};

class ProxyObjectsCaches: public virtual DanglingBaseClass {
public:
    void add(
        RemoteSiteId proxy_id,
        RemoteObjectId object_id,
        std::unique_ptr<ProxyObjectCacheEntry>&& cache);
    ProxyObjectCacheEntry* try_get(RemoteSiteId proxy_id, RemoteObjectId object_id);
    template <class T>
    T* try_get(RemoteSiteId proxy_id, RemoteObjectId object_id) {
        auto* o = try_get(proxy_id, object_id);
        if (o == nullptr) {
            return nullptr;
        }
        auto* result = dynamic_cast<T*>(o);
        if (result == nullptr) {
            throw std::runtime_error("Proxy object cache has the wrong type");
        }
        return result;
    }
    template <class T>
    T& get_or_create(RemoteSiteId proxy_id, RemoteObjectId object_id) {
        auto* res = try_get<T>(proxy_id, object_id);
        if (res == nullptr) {
            auto ures = std::make_unique<T>();
            res = ures.get();
            add(proxy_id, object_id, std::move(ures));
        }
        return *res;
    }
    void remove_proxy(RemoteSiteId proxy_id);
    void remove_object(RemoteObjectId object_id);
    void print_status(std::ostream& ostr) const;
private:
    std::unordered_map<ChannelPair, std::unique_ptr<ProxyObjectCacheEntry>, ChannelPairHash> cache_registry_;
    std::unordered_multimap<RemoteSiteId, ChannelPair> proxy_index_;    // Key: proxy_id
    std::unordered_multimap<RemoteObjectId, ChannelPair> object_index_; // Key: object_id
};

}
