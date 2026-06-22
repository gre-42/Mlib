#pragma once
#include <Mlib/Misc/Object.hpp>
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>

namespace Mlib {

class SerializationContextRead {
public:
    SerializationContextRead() {
        objects_.emplace(0, nullptr);
    }
    ~SerializationContextRead() = default;
    template <class T>
    inline std::optional<std::shared_ptr<T>> try_get(uint32_t index) {
        auto res = objects_.find(index);
        if (res == objects_.end()) {
            return std::nullopt;
        }
        if (res->second == nullptr) {
            return nullptr;
        }
        auto res_t = std::dynamic_pointer_cast<T>(res->second);
        if (res_t == nullptr) {
            throw std::runtime_error("Pointer has incorrect type");
        }
        return res_t;
    }
    uint32_t add(std::shared_ptr<Object> obj) {
        auto res = objects_.try_emplace(objects_.size(), std::move(obj));
        if (!res.second) {
            throw std::runtime_error("Could not insert object");
        }
        return res.first->first;
    }
private:
    std::unordered_map<uint32_t, std::shared_ptr<Object>> objects_;
};

}
