// bench.cpp — memory-pool vs malloc 性能对比
#include "fixed_allocator.h"
#include "slab_allocator.h"
#include <chrono>
#include <iostream>
#include <cstdlib>   // rand, srand, malloc, free

int main() {
    const int N = 1000000;  // 100 万次
    using clock = std::chrono::steady_clock;

    // ====== FixedAllocator<32> vs malloc ======
    {
        FixedAllocator<32> alloc(N);
        auto t1 = clock::now();
        for (int i = 0; i < N; i++) {
            void* p = alloc.allocate();
            alloc.deallocate(p);
        }
        auto t2 = clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout << "FixedAllocator<32>: " << N / 1000 << "K alloc+free in "
                  << ms << "ms -> " << (N * 1000LL / (ms ? ms : 1)) << " ops/s\n";
    }

    {
        auto t1 = clock::now();
        for (int i = 0; i < N; i++) {
            void* p = ::malloc(32);
            ::free(p);
        }
        auto t2 = clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout << "malloc(32):        " << N / 1000 << "K alloc+free in "
                  << ms << "ms -> " << (N * 1000LL / (ms ? ms : 1)) << " ops/s\n";
    }

    // ====== SlabAllocator(random size) vs malloc(random size) ======
    {
        SlabAllocator alloc;
        srand(42);  // 固定种子，公平对比
        auto t1 = clock::now();
        for (int i = 0; i < N; i++) {
            size_t sz = (rand() % 200) + 1;
            void* p = alloc.allocate(sz);
            alloc.deallocate(p, sz);  // 立即回收，不攒着
        }
        auto t2 = clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout << "SlabAllocator:     " << N / 1000 << "K alloc+free in "
                  << ms << "ms -> " << (N * 1000LL / (ms ? ms : 1)) << " ops/s\n";
    }

    {
        srand(42);  // 同样的种子，公平对比
        auto t1 = clock::now();
        for (int i = 0; i < N; i++) {
            size_t sz = (rand() % 200) + 1;
            void* p = ::malloc(sz);
            ::free(p);
        }
        auto t2 = clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout << "malloc(random):    " << N / 1000 << "K alloc+free in "
                  << ms << "ms -> " << (N * 1000LL / (ms ? ms : 1)) << " ops/s\n";
    }
}
