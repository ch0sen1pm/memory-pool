#pragma once
#include "fixed_allocator.h"
#include <mutex>

template <size_t BlockSize>
class CentralCache {
public:
    void* allocate() {
        std::lock_guard<std::mutex> lock(mutex_);
        return alloc_.allocate();
    }

    void deallocate(void* p) {
        std::lock_guard<std::mutex> lock(mutex_);
        alloc_.deallocate(p);
    }

private:
    FixedAllocator<BlockSize> alloc_;
    std::mutex mutex_;
};