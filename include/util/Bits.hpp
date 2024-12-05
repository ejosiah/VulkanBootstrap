#pragma once

#include <Types.hpp>

namespace bits {

    constexpr uint32 Byte = 8;

    template<typename T>
    requires std::is_integral_v<T>
    auto findHighestBitSet(T bitset) {
        auto size = sizeof(T) * Byte;
        for(int pos = size - T(1); pos >=  0; --pos) {
            T bit = (bitset >> pos) & 1;

            if(bit == T(1)){
                return T(1) << pos;
            }
        }
        return T();
    }

    template<typename T>
    requires std::is_integral_v<T>
    auto count(T bitset) {
        auto size = sizeof(T) * Byte;
        auto sum = 0;
        for(auto i = 0; i < size; ++i){
            sum += (bitset >> i) & 1;
        }
        return sum;
    }
}