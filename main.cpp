#include "central_cache.h"
#include <iostream>
#include <thread>
#include <cassert>

int main() {
    CentralCache<32> pool;

    // 4 个线程争用同一个 CentralCache（有锁）
    std::thread t1([&]() {
        for (int i = 0; i < 1000; i++) {
            void* p = pool.allocate();
            assert(p != nullptr);
            pool.deallocate(p);
        }
        std::cout << "[t1] done\n";
    });

    std::thread t2([&]() {
        for (int i = 0; i < 1000; i++) {
            void* p = pool.allocate();
            assert(p != nullptr);
            pool.deallocate(p);
        }
        std::cout << "[t2] done\n";
    });

    std::thread t3([&]() {
        for (int i = 0; i < 1000; i++) {
            void* p = pool.allocate();
            assert(p != nullptr);
            pool.deallocate(p);
        }
        std::cout << "[t3] done\n";
    });

    std::thread t4([&]() {
        for (int i = 0; i < 1000; i++) {
            void* p = pool.allocate();
            assert(p != nullptr);
            pool.deallocate(p);
        }
        std::cout << "[t4] done\n";
    });

    t1.join(); t2.join(); t3.join(); t4.join();
    std::cout << "CentralCache: 4 threads OK (with mutex)\n";
}
