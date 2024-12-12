#include <cassert>
#include <numeric>

template<typename T>
constexpr Pooled<T>::Pooled(T *ptr, ResourceHandle handle) noexcept
        : ptr_(ptr)
        , handle_(handle){}

template<typename T>
Pooled<T>::Reference Pooled<T>::operator*() const noexcept {
    return *ptr_;
}

template<typename T>
Pooled<T>::Pointer Pooled<T>::operator->() const noexcept {
    return ptr_;
}

template<typename T>
ResourceHandle Pooled<T>::handle() const {
    return handle_;
}


template<typename T>
ResourcePool<T>::ResourcePool(size_t capacity)
        : pool_(capacity)
        , freeIndices_(capacity)
        , capacity_(capacity){
    std::iota(freeIndices_.begin(), freeIndices_.end(), 0);
    freeIndicesHead_ = freeIndices_.begin();
}

template<typename T>
ResourcePool<T>::~ResourcePool() {
    releaseAl();
    pool_.clear();
    freeIndices_.clear();
}

template<typename T>
ResourceHandle ResourcePool<T>::acquire() {
    assert(freeIndicesHead_ != freeIndices_.end());
    auto index = *freeIndicesHead_;
    ++freeIndicesHead_;
    return index;
}

template<typename T>
Pooled<T> ResourcePool<T>::acquireResource() {
    return get(acquire());
}

template<typename T>
Pooled<T> ResourcePool<T>::get(ResourceHandle handle) const {
    assert(handle >= 0 && handle < capacity_);
    return { &pool_[handle], handle };
}

template<typename T>
void ResourcePool<T>::releaseAl() {
    std::iota(freeIndices_.begin(), freeIndices_.end(), 0);
    freeIndicesHead_ = freeIndices_.begin();
}

template<typename T>
void ResourcePool<T>::release(Pooled<T> resource) {
    release(resource.handle());
}

template<typename T>
void ResourcePool<T>::release(ResourceHandle handle) {
    assert(std::distance(freeIndices_.begin(), freeIndicesHead_) >= 0);
    --freeIndicesHead_;
    *freeIndicesHead_ = handle;
}