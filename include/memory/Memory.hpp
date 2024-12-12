#pragma once

#include "Types.hpp"
#include <cstdlib>
#include <cstdio>
#include <spdlog/spdlog.h>
#include <tlsf.h>

#define ALLOCATOR_MEMBER_TYPES(Type) \
    using value_type =	Type; \
    using pointer =	Type*;\
    using const_pointer =	const Type*;\
    using reference =	Type&;\
    using const_reference =	const Type&;\
    using size_type =	std::size_t;\
    using difference_type =	std::ptrdiff_t;\
    using propagate_on_container_move_assignment =	std::true_type;\
    using is_always_equal = std::true_type;\
    template< class U >\
    struct rebind {\
        using other = HeapAllocator<U>;\
    }; \
    size_t alignment = sizeof(Type);

struct MemoryStats {
    size_t free{};
    size_t used{};

    static MemoryStats& instance() {
        static MemoryStats stats{};
        return stats;
    }
    std::vector<int> v;
};

constexpr size_t SYSTEM_MEMORY_CAPACITY = (2u << 30); // 2 GB

struct SystemMemory{

    explicit SystemMemory(size_t capacity);

    void* allocate(size_t size);

    void deallocate(void* p);

    static SystemMemory& instance();

private:
    void* tlsfHandle_{};
    void* memory_{};
    size_t allocatedSize_{};
    size_t capacity_{};
};


template<typename T>
struct HeapAllocator{

    ALLOCATOR_MEMBER_TYPES(T)

    explicit HeapAllocator(size_t capacity);

    constexpr T* allocate( std::size_t n );

    void deallocate( T* p, std::size_t n );

private:
    void* memory_{};
    void* tlsfHandle_{};
    size_t capacity_{};
    size_t allocatedSize_{};
};

template<typename T>
HeapAllocator<T>::HeapAllocator(size_t capacity)
        : memory_(malloc(capacity))
        , capacity_(capacity)
{
    tlsfHandle_ = tlsf_create_with_pool(memory_, capacity);
}

template<typename T>
constexpr T *HeapAllocator<T>::allocate(std::size_t n) {
    void* allocation = nullptr;
    if(alignment == 1){
        allocation = tlsf_malloc(tlsfHandle_, n);
    }else {
        allocation = tlsf_memalign(tlsfHandle_, alignment, n);
    }
    auto actualSize = tlsf_block_size( allocation );
    allocatedSize_ += actualSize;

    return allocation;
}

template<typename T>
void HeapAllocator<T>::deallocate(T *p, std::size_t n) {
    size_t actualSize = tlsf_block_size(p);
    allocatedSize_ -= actualSize;
    tlsf_free(*p);
}