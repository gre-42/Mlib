#pragma once
#include <Mlib/Remote/Incremental_Objects/Proxy_Object_Cache.hpp>

namespace Mlib {

class IncrementalCacheObjectToken {
    IncrementalCacheObjectToken(const IncrementalCacheObjectToken&) = delete;
    IncrementalCacheObjectToken& operator = (const IncrementalCacheObjectToken&) = delete;
public:
    IncrementalCacheObjectToken(
        const DanglingBaseClassRef<ProxyObjectsCaches>& proxy_object_cache,
        RemoteObjectId object_id)
        : proxy_object_cache_{proxy_object_cache}
        , object_id_{object_id}
    {}
    ~IncrementalCacheObjectToken() {
        proxy_object_cache_->remove_object(object_id_);
    }
private:
    DanglingBaseClassRef<ProxyObjectsCaches> proxy_object_cache_;
    RemoteObjectId object_id_;
};

}
