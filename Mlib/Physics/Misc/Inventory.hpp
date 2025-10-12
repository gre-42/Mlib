#pragma once
#include <Mlib/Physics/Misc/Inventory_Item.hpp>
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
    bool knows_item_type(const InventoryItem& item_type) const;
    void set_capacity(const InventoryItem& item_type, uint32_t value);
    uint32_t navailable(const InventoryItem& item_type) const;
    uint32_t nfree(const InventoryItem& item_type) const;
    void add(const InventoryItem& item_type, uint32_t amount);
    void take(const InventoryItem& item_type, uint32_t amount);
private:
    const ItemInfo& item(const InventoryItem& item_type) const;
    ItemInfo& item(const InventoryItem& item_type);
    std::unordered_map<InventoryItem, ItemInfo> items_;
};

}
