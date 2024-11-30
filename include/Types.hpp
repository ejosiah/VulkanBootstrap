#pragma once

#include <cinttypes>

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template<typename T>
constexpr T to(auto obj) {
    return static_cast<T>(obj);
}

template<typename T>
constexpr T as(auto obj) {
    // TODO make sure object if pointer or ref
    return reinterpret_cast<T*>(obj);
}

using int32 = uint32_t;
using uint32 = uint32_t;