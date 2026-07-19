#include "thread_cache.h"
#include <iostream>
#include <thread>
#include <vector>
#include <cassert>

int main() {
    ThreadCache<32> cache;

    // 两个线程并发分配/回收 1000 次——同一个 cache 对象，无锁
    std::thread t1([&]() {
        for (int i = 0; i < 1000; i++) {
            void* p = cache.allocate();
            assert(p != nullptr);
            cache.deallocate(p);
        }
        std::cout << "thread " << std::this_thread::get_id()
                  << ": 1000 alloc+free OK\n";
    });

    std::thread t2([&]() {
        for (int i = 0; i < 1000; i++) {
            void* p = cache.allocate();
            assert(p != nullptr);
            cache.deallocate(p);
        }
        std::cout << "thread " << std::this_thread::get_id()
                  << ": 1000 alloc+free OK\n";
    });

    t1.join();
    t2.join();

    std::cout << "ThreadCache: both threads completed without lock\n";
}
