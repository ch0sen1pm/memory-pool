#include "thread_cache.h"
#include <iostream>
#include <thread>
#include <vector>
#include <cassert>

int main() {
    ThreadCache<32> cache;

    // 两个线程并发分配/回收——同一个 cache 对象，无锁
    std::thread t1([&]() {
        for (int i = 0; i < 5; i++) {
            // 分配 + 回收重复 1000 次
            for (int j = 0; j < 1000; j++) {
                void* p = cache.allocate();
                cache.deallocate(p);
            }
            std::cout << "[t1] batch " << i+1 << " done\n";
        }
    });

    std::thread t2([&]() {
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 1000; j++) {
                void* p = cache.allocate();
                cache.deallocate(p);
            }
            std::cout << "[t2] batch " << i+1 << " done\n";
        }
    });

    t1.join();
    t2.join();

    std::cout << "ThreadCache: both threads completed without lock\n";
}
