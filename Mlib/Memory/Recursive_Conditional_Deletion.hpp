#pragma once
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

template <class TContainer, class TPredicate>
void remove_from_map_recursively_if(TContainer& elements, const TPredicate& predicate) {
    TContainer map2;
    for (auto it = elements.cbegin(); it != elements.cend();) {
        if (predicate(*it)) {
            auto node = elements.extract(it++);
            if (!map2.insert(std::move(node)).inserted) {
                THROW_OR_ABORT("Map element added during deletion, and deleted right again");
            }
        } else {
            ++it;
        }
    }
}

}
