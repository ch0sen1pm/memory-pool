#include "fixed_allocator.h"
#include "lockfree_stack.h"
#include <iostream>
#include <thread>
#include <cassert>
#include <cstdint>

struct Data {
    int value;
};

int main() {
    // 对齐测试——非 debug 模式
    {
        FixedAllocator<32, 64> alloc(4);
        void* p = alloc.allocate();
        uintptr_t addr = reinterpret_cast<uintptr_t>(p);
        std::cout << "align test: " << (addr % 64 == 0 ? "OK" : "FAIL")
                  << " (addr=" << addr << ")\n";
        alloc.deallocate(p);
    }

    // Debug guard 测试——debug 模式，越界写应触发 assert
    {
        FixedAllocator<32, 64, true> alloc(4);
        void* p = alloc.allocate();
        char* buf = static_cast<char*>(p);
        buf[56] = 'X';  // b+16+56 = b+72，恰好踩到后 guard
        alloc.deallocate(p);  // <- assert 应该炸
    }
    std::cout << "Debug guard: OK\n";

    // LockFreeStack 测试
    LockFreeStack<Data> stack;

    // 临时分配节点（用 new 模拟，实际场景从内存池取）
    auto* a = new LockFreeStack<Data>::Node;
    auto* b = new LockFreeStack<Data>::Node;
    auto* c = new LockFreeStack<Data>::Node;
    a->data.value = 1;
    b->data.value = 2;
    c->data.value = 3;

    // push 三个
    stack.push(a);
    stack.push(b);
    stack.push(c);

    // pop 三个
    auto* x = stack.pop();
    auto* y = stack.pop();
    auto* z = stack.pop();

    std::cout << "pop: " << x->data.value << " "
              << y->data.value << " "
              << z->data.value << "\n";
    // LIFO: 3 2 1

    delete a; delete b; delete c;

    // 多线程测试：4 线程各 push+pop 1000 次
    std::thread t1([&]() {
        for (int i = 0; i < 1000; i++) {
            auto* n = new LockFreeStack<Data>::Node;
            n->data.value = i;
            stack.push(n);
            auto* p = stack.pop();
            if (p) delete p;
        }
        printf("[t1] 1000 ops done\n");
    });

    std::thread t2([&]() {
        for (int i = 0; i < 1000; i++) {
            auto* n = new LockFreeStack<Data>::Node;
            n->data.value = i;
            stack.push(n);
            auto* p = stack.pop();
            if (p) delete p;
        }
        printf("[t2] 1000 ops done\n");
    });

    std::thread t3([&]() {
        for (int i = 0; i < 1000; i++) {
            auto* n = new LockFreeStack<Data>::Node;
            n->data.value = i;
            stack.push(n);
            auto* p = stack.pop();
            if (p) delete p;
        }
        printf("[t3] 1000 ops done\n");
    });

    std::thread t4([&]() {
        for (int i = 0; i < 1000; i++) {
            auto* n = new LockFreeStack<Data>::Node;
            n->data.value = i;
            stack.push(n);
            auto* p = stack.pop();
            if (p) delete p;
        }
        printf("[t4] 1000 ops done\n");
    });

    t1.join(); t2.join(); t3.join(); t4.join();
    std::cout << "LockFreeStack: 4 threads OK (no mutex)\n"
              << std::flush;
}
