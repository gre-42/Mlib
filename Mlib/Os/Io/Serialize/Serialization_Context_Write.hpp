#pragma once
#include <Mlib/Misc/Object.hpp>
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace Mlib {

class SerializationContextWrite {
public:
    inline std::pair<uint32_t, bool> add_or_get(const std::shared_ptr<Object>& obj) {
        auto res = objects_.try_emplace(obj, objects_.size());
        return {res.first->second, res.second};
    }
private:
    std::unordered_map<std::shared_ptr<Object>, uint32_t> objects_;
};

}
