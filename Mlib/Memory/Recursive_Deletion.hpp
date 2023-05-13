#pragma once

namespace Mlib {

template <class TContainer>
void clear_container_recursively(TContainer& container) {
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
void clear_set_recursively(TContainer& container, const TFunction& deleter) {
    while (!container.empty()) {
        auto node = container.extract(container.begin());
        deleter(node.value());
    }
}

template <class TContainer, class TFunction>
void clear_list_recursively(TContainer& elements, const TFunction& deleter) {
    while (!elements.empty()) {
        TContainer list2;
        auto it_second = elements.begin();
        ++it_second;
        list2.splice(list2.begin(), elements, elements.begin(), it_second);
        deleter(list2.front());
    }
}

}
