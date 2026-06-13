#pragma once
#include <Mlib/Remote/Incremental_Objects/Proxy_Object_Cache.hpp>

namespace Mlib {

class IncrementalCacheProxyToken {
    IncrementalCacheProxyToken(const IncrementalCacheProxyToken&) = delete;
    IncrementalCacheProxyToken& operator = (const IncrementalCacheProxyToken&) = delete;
public:
    IncrementalCacheProxyToken(
        const DanglingBaseClassRef<ProxyObjectsCaches>& proxy_object_cache,
        RemoteSiteId proxy_id)
        : proxy_object_cache_{proxy_object_cache}
        , proxy_id_{proxy_id}
    {}
    ~IncrementalCacheProxyToken() {
        proxy_object_cache_->remove_proxy(proxy_id_);
    }
private:
    DanglingBaseClassRef<ProxyObjectsCaches> proxy_object_cache_;
    RemoteSiteId proxy_id_;
};

}
