#pragma once

#include "Types.hpp"
#include "memory/Memory.hpp"
#include <memory>

template<typename T>
class Pooled {
    using Pointer = T*;
    using Reference = T&;
public:
    constexpr explicit Pooled(T* ptr, ResourceHandle handle) noexcept;

    inline Reference operator*() const noexcept;
    inline Pointer operator->() const noexcept;
    [[nodiscard]] inline ResourceHandle handle() const;

private:
    T* ptr_;
    ResourceHandle handle_{};
};

template <typename T>
class ResourcePool {
public:
    explicit ResourcePool(size_t capacity);

    ~ResourcePool();

    Pooled<T> acquireResource();

    ResourceHandle acquire();

    Pooled<T> get(ResourceHandle handle) const;

    void release(ResourceHandle handle);

    void release(Pooled<T> resource);

    void releaseAl();

private:
    std::vector<T> pool_;
    std::vector<ResourceHandle> freeIndices_;
    std::vector<ResourceHandle>::iterator freeIndicesHead_{};
    size_t capacity_;
};

#include "ResourcePool.inl"
