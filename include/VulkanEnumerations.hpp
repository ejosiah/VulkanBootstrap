#pragma once

#include <volk.h>

#include <vector>
#include <utility>

template<typename EntryType>
auto v_enumerate(auto enumerator, auto source) {
    std::vector<EntryType> values;

    uint32_t count;
    enumerator(source, &count, nullptr);
    std::vector<EntryType> list(count);
    enumerator(source, &count, list.data());

    return list;
}