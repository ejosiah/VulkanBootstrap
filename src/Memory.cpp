#include "memory/Memory.hpp"

SystemMemory::SystemMemory(size_t capacity)
: memory_(malloc(capacity))
, capacity_(capacity){
    tlsfHandle_ = tlsf_create_with_pool(memory_, capacity);
}

void *SystemMemory::allocate(size_t size) {
    return tlsf_malloc(tlsfHandle_, size);
}

void SystemMemory::deallocate(void *p) {
    tlsf_free(tlsfHandle_, p);
}

SystemMemory &SystemMemory::instance() {
    static auto sm = SystemMemory(SYSTEM_MEMORY_CAPACITY);
    return sm;
}

void * operator new(size_t size) {
    static auto& sm = SystemMemory::instance();
    auto allocation = sm.allocate(size);
    return allocation;
}


void operator delete(void *allocation) noexcept {
    static auto& sm = SystemMemory::instance();
    sm.deallocate(allocation);
}