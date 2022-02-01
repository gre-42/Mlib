#pragma once

namespace Mlib {

template <class TContainer>
void clear_map_recursively(TContainer& container) {
    while (!container.empty()) {
        container.erase(container.begin());
    }
}

template <class TContainer, class TFunction>
void clear_map_recursively(TContainer& container, const TFunction& deleter) {
    while (!container.empty()) {
        auto node = container.extract(container.begin());
        deleter(node);
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
