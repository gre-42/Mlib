#pragma once

namespace Mlib {

template <class TContainer>
void clear_map_recursively(TContainer& container) {
    while (!container.empty()) {
        auto it = container.begin();
        auto value = std::move(it->second);
        container.erase(it);
    }
}

template <class TContainer, class TFunction>
void delete_elements_recursively(TContainer& elements, const TFunction& deleter) {
    while (!elements.empty()) {
        auto element_it = elements.begin();
        auto element_value = *element_it;
        elements.erase(element_it);
        deleter(element_value);
    }
}

}
