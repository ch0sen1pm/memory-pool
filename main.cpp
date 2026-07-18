#include "fixed_allocator.h"
#include <cassert>
#include <iostream>

int main() {
    FixedAllocator<32> alloc(4);

    void* a = alloc.allocate();
    void* b = alloc.allocate();
    alloc.deallocate(a);
    void* c = alloc.allocate();
    assert(a == c);
    std::cout << "alloc == dealloc -> reuse OK\n";
}