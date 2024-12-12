#pragma once

#include <cinttypes>
#include <type_traits>
#include <vector>

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template<typename T>
constexpr T to(auto obj) {
    return static_cast<T>(obj);
}

template<typename T>
constexpr auto as(auto obj) {
    if constexpr (std::is_pointer<decltype(obj)>::value) {
        return reinterpret_cast<T *>(obj);
    } else if constexpr (std::is_reference_v<decltype(obj)>) {
        return reinterpret_cast<T&>(obj);
    }
}

using int8 = int8_t;
using uint8 = uint8_t;
using int16 = int16_t;
using uint16 = uint16_t;
using int32 = uint32_t;
using uint32 = uint32_t;
using int64 = int64_t;
using uint64 = uint64_t;

using ResourceHandle = uint32;