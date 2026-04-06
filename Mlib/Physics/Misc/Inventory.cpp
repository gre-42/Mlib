
#include "Inventory.hpp"
#include <stdexcept>

using namespace Mlib;

Inventory::Inventory()
{}

Inventory::~Inventory()
{}

bool Inventory::knows_item_type(const InventoryItem& item_type) const {
    return items_.contains(item_type);
}

const ItemInfo& Inventory::item(const InventoryItem& item_type) const {
    auto it = items_.find(item_type);
    if (it == items_.end()) {
        throw std::runtime_error("Unknown item type: \"" + *item_type + '"');
    }
    return it->second;
}

ItemInfo& Inventory::item(const InventoryItem& item_type) {
    const Inventory* cthis = this;
    return const_cast<ItemInfo&>(cthis->item(item_type));
}

void Inventory::set_capacity(const InventoryItem& item_type, uint32_t value) {
    if (!items_.insert({item_type, ItemInfo{.capacity = value, .navailable = 0}}).second) {
        throw std::runtime_error("Capacity already set for item \"" + *item_type + '"');
    }
}

uint32_t Inventory::navailable(const InventoryItem& item_type) const {
    return item(item_type).navailable;
}

uint32_t Inventory::nfree(const InventoryItem& item_type) const {
    const auto& itm = item(item_type);
    return itm.capacity - itm.navailable;
}

void Inventory::add(const InventoryItem& item_type, uint32_t amount) {
    auto& itm = item(item_type);
    if (itm.navailable + amount > itm.capacity) {
        throw std::runtime_error("Inventory not large enough");
    }
    itm.navailable += amount;
}

void Inventory::take(const InventoryItem& item_type, uint32_t amount) {
    auto& itm = item(item_type);
    if (itm.navailable < amount) {
        throw std::runtime_error("Insufficient items: \"" + *item_type + '"');
    }
    itm.navailable -= amount;
}
