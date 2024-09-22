#pragma once
#include <Mlib/Threads/Unlock_Guard.hpp>

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

template <class TContainer, class TLock, class TFunction>
void clear_set_recursively_with_lock(
    TContainer& container,
    TLock& lock,
    const TFunction& deleter)
{
    while (!container.empty()) {
        auto node = container.extract(container.begin());
        UnlockGuard ulock{ lock };
        deleter(node.value());
    }
}

template <class TContainer, class TFunction>
void clear_list_recursively(TContainer& elements, const TFunction& deleter) {
    while (!elements.empty()) {
        auto it_second = elements.begin();
        ++it_second;
        TContainer list2;
        list2.splice(list2.begin(), elements, elements.begin(), it_second);
        deleter(list2.front());
    }
}

template <class TContainer, class TLock, class TFunction>
void clear_list_recursively_with_lock(
    TContainer& elements,
    TLock& lock,
    const TFunction& deleter)
{
    while (!elements.empty()) {
        auto it_second = elements.begin();
        ++it_second;
        TContainer list2;
        list2.splice(list2.begin(), elements, elements.begin(), it_second);
        UnlockGuard ulock{ lock };
        deleter(list2.front());
        list2.clear();
    }
}

}
