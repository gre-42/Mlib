#pragma once
#include <iostream>
#include <map>

namespace Mlib {

class UUID {
public:
    friend std::ostream& operator << (std::ostream& ostr, const UUID& uuid);
    UUID() {};
    inline explicit UUID(size_t uuid)
    : uuid_(uuid)
    {}
    inline bool operator < (const UUID& rhs) const {
        return uuid_ < rhs.uuid_;
    }
    size_t uuid_;
};

inline std::ostream& operator << (std::ostream& ostr, const UUID& uuid) {
    ostr << "U(" << uuid.uuid_ << ")";
    return ostr;
}

template <class TCameraVariable, class TFeaturePointVariable>
class UUIDGen {
public:
    inline UUID get(const TCameraVariable& ci) const {
        return cameras_.at(ci);
    }
    inline UUID get(const TFeaturePointVariable& fi) const {
        return feature_points_.at(fi);
    }
    inline void generate(const TCameraVariable& ci) {
        if (cameras_.find(ci) == cameras_.end()) {
            cameras_.insert(std::make_pair(ci, UUID(last_uuid_++)));
        }
    }
    inline void generate(const TFeaturePointVariable& fi) {
        if (feature_points_.find(fi) == feature_points_.end()) {
            feature_points_.insert(std::make_pair(fi, UUID(last_uuid_++)));
        }
    }
private:
    std::map<TCameraVariable, UUID> cameras_;
    std::map<TFeaturePointVariable, UUID> feature_points_;
    size_t last_uuid_ = 0;
};

}
