#pragma once
#include "fixed_allocator.h"
#include <thread>

template <size_t BlockSize>
class ThreadCache {
    static thread_local FixedAllocator<BlockSize> alloc_;

public:
    void* allocate() {
        return alloc_.allocate();
    }

    void deallocate(void* p) {
        alloc_.deallocate(p);
    }
};

template <size_t BlockSize>
thread_local FixedAllocator<BlockSize> ThreadCache<BlockSize>::alloc_;
