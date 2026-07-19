// bench_mt.cpp — 多线程 FixedAllocator vs malloc
#include "fixed_allocator.h"
#include "thread_cache.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>
#include <cstdlib>

int main() {
    const int N = 100000;  // 每线程 10 万次
    const int T = 4;       // 4 个线程
    using clock = std::chrono::steady_clock;

    // ====== FixedAllocator<32> + mutex ======
    {
        FixedAllocator<32> alloc(N * T);
        std::mutex mtx;

        auto t1 = clock::now();
        std::thread ts[T];
        for (int t = 0; t < T; t++) {
            ts[t] = std::thread([&]() {
                for (int i = 0; i < N; i++) {
                    void* p;
                    { std::lock_guard<std::mutex> lk(mtx); p = alloc.allocate(); }
                    { std::lock_guard<std::mutex> lk(mtx); alloc.deallocate(p); }
                }
            });
        }
        for (auto& t : ts) t.join();
        auto t2 = clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        auto ops = (N * T * 1000LL) / (ms ? ms : 1);
        std::cout << "FixedAllocator<32> " << T << " threads: "
                  << ms << "ms -> " << ops << " ops/s\n";
    }

    // ====== malloc(32) + mutex ======
    {
        std::mutex mtx;

        auto t1 = clock::now();
        std::thread ts[T];
        for (int t = 0; t < T; t++) {
            ts[t] = std::thread([&]() {
                for (int i = 0; i < N; i++) {
                    void* p;
                    { std::lock_guard<std::mutex> lk(mtx); p = ::malloc(32); }
                    { std::lock_guard<std::mutex> lk(mtx); ::free(p); }
                }
            });
        }
        for (auto& t : ts) t.join();
        auto t2 = clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        auto ops = (N * T * 1000LL) / (ms ? ms : 1);
        std::cout << "malloc(32)        " << T << " threads: "
                  << ms << "ms -> " << ops << " ops/s\n";
    }

    // ====== ThreadCache<32> — 无锁版 ======
    {
        ThreadCache<32> cache;

        auto t1 = clock::now();
        std::thread ts[T];
        for (int t = 0; t < T; t++) {
            ts[t] = std::thread([&]() {
                for (int i = 0; i < N; i++) {
                    void* p = cache.allocate();
                    cache.deallocate(p);
                }
            });
        }
        for (auto& t : ts) t.join();
        auto t2 = clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        auto ops = (N * T * 1000LL) / (ms ? ms : 1);
        std::cout << "ThreadCache<32>   " << T << " threads: "
                  << ms << "ms -> " << ops << " ops/s  (no lock!)\n";
    }
}
