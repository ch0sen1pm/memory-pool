#include "slab_allocator.h"
#include <cassert>
#include <iostream>

int main() {
    SlabAllocator alloc;

    // 不同大小的分配请求
    void* p1 = alloc.allocate(10);   // roundUp→16 → slab16
    void* p2 = alloc.allocate(50);   // roundUp→64 → slab64
    void* p3 = alloc.allocate(200);  // roundUp→256 → slab256

    alloc.deallocate(p1, 10);
    void* p4 = alloc.allocate(12);   // roundUp→16，应复用 p1

    assert(p1 == p4);  // 16 字节的 freelist 回收复用
    alloc.deallocate(p2, 50);
    alloc.deallocate(p3, 200);

    std::cout << "SlabAllocator: multi-size alloc/dealloc OK\n";
}