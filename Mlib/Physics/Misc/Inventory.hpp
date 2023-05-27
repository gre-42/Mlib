#pragma once
#include <cstdint>
#include <map>
#include <string>

namespace Mlib {

struct ItemInfo {
    uint32_t capacity;
    uint32_t navailable;
};

class Inventory {
public:
    explicit Inventory();
    ~Inventory();
    bool knows_item_type(const std::string& item_type) const;
    void set_capacity(const std::string& item_type, uint32_t value);
    uint32_t navailable(const std::string& item_type) const;
    uint32_t nfree(const std::string& item_type) const;
    void add(const std::string& item_type, uint32_t amount);
    void take(const std::string& item_type, uint32_t amount);
private:
    const ItemInfo& item(const std::string& item_type) const;
    ItemInfo& item(const std::string& item_type);
    std::map<std::string, ItemInfo> items_;
};

}
