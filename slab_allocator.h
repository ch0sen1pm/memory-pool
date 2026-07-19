#pragma once
#include "fixed_allocator.h"
#include <cstddef>
#include <cstdlib>

constexpr size_t roundUp(size_t n) {
    n --;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;

    return n + 1;
}

class SlabAllocator {
    FixedAllocator<8> slab8_;
    FixedAllocator<16> slab16_;
    FixedAllocator<32> slab32_;
    FixedAllocator<64> slab64_;
    FixedAllocator<128> slab128_;
    FixedAllocator<256> slab256_;

public:
    void* allocate(size_t size) {
        size_t rounded = roundUp(size);
        if (rounded <= 8)   return slab8_.allocate();
        if (rounded <= 16)  return slab16_.allocate();
        if (rounded <= 32)  return slab32_.allocate();
        if (rounded <= 64)  return slab64_.allocate();
        if (rounded <= 128) return slab128_.allocate();
        if (rounded <= 256) return slab256_.allocate();
        return ::malloc(size);
    }

    void deallocate(void* p, size_t size) {
          size_t rounded = roundUp(size);
          if (rounded <= 8)   slab8_.deallocate(p);
          else if (rounded <= 16)  slab16_.deallocate(p);
          else if (rounded <= 32)  slab32_.deallocate(p);
          else if (rounded <= 64)  slab64_.deallocate(p);
          else if (rounded <= 128) slab128_.deallocate(p);
          else if (rounded <= 256) slab256_.deallocate(p);
          else ::free(p);  // 之前是 malloc 的，现在 free
    }
};