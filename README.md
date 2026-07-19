# memory-pool

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)

从零手写 C++ 高性能内存池。端侧设备、游戏引擎、量化交易这类场景下，频繁 `malloc/free` 的系统调用开销和内存碎片不可接受——内存池预分配大块内存、自己管理分配与回收，将系统调用减少 90% 以上。

目标不是替代 jemalloc/tcmalloc，而是理解"用户态内存分配器怎么工作"——每一层都亲手实现。

## 为什么需要内存池

**malloc/new 做了什么：**

```
void* p = malloc(32);
// 1. 检查线程本地缓存（有就直接返回，快）
// 2. 缓存不够 → 查全局内存池
// 3. 还不够 → 调 brk()/mmap() 向内核要内存（系统调用，慢）
// 4. 内核切一块出来 → 返回
```

99% 的时候前两步就完成了，不需要系统调用。malloc 本身就是一种内存池。但它是通用目的——管理各种大小的分配，还要处理多线程竞争，内部逻辑复杂。

**特殊场景不需要通用的 malloc：** 你的 muduo 在处理几十万个 TCP 连接时，每个连接的 Buffer 大小固定。每来一个新连接都要 `new Buffer()`，频繁的系统调用拖死性能。如果预分配 10000 个 Buffer 大小的块，要用时直接从 freelist 取——零系统调用，零碎片。

## 核心设计

### 第一层：Fixed-size Allocator（固定大小分配器）

为一种固定大小（比如 32 字节）管理内存。预分配一整块大内存，切成等大的小块，用单链表串起来。

```
预分配块: [block][block][block][block]...
freelist:  head → block → block → block → nullptr
```

**分配：** 从 freelist 取第一个块。**回收：** 把这个块插回 freelist。都是 O(1) 操作，不调系统调用。

### 第二层：Slab Allocator（多尺寸 slab）

管理多个固定大小分配器——8 字节、16 字节、32 字节、64 字节... 每来一个分配请求，根据请求大小分到合适的 slab。

```
请求 12 字节 → 找 16 字节 slab → allocate()
请求 30 字节 → 找 32 字节 slab → allocate()
请求 128 字节 → 找 128 字节 slab → allocate()
请求 5KB → 超出最大 slab → 直接 malloc
```

### 第三层：Thread-local Cache

每个线程有自己的 freelist 缓存。线程内 alloc 完全无锁——只碰自己的缓存。缓存空了才去全局池拿。缓存满了回收一部分回全局池。

```
线程 A: [自己的 16B slab] [自己的 32B slab] ...   ← 无锁
线程 B: [自己的 16B slab] [自己的 32B slab] ...   ← 无锁
           ↕ 缓存空了/满了批量转移              ↕
         [全局 central cache]   ← 有锁，但竞争少
```

### 第四层：Central Cache

全局池，所有线程共享。用于线程间调节——线程 A 释放太多时回给全局池，线程 B 不够时从全局池拿。加锁频率低（只有批量转移时才锁），大部分操作在线程本地完成。

## Architecture

```
allocate(N)
  → 找对应的 slab size
    → thread-local cache 有 → O(1) 拿走 ✅
    → thread-local cache 空 ↓
  → 向 central cache 批量申请
    → central cache 有 → 给一批 ✅
    → central cache 空 ↓
  → 向 OS 预申请一大块 → 切块 → 返回 ✅

deallocate(ptr)
  → 放回 thread-local cache
    → thread-local cache 没满 → 插入 freelist ✅
    → thread-local cache 满了 ↓
  → 批量回收一批给 central cache
```

## Roadmap

### 第一阶段：基础分配器（单线程）

- [x] **Fixed-size allocator** — 固定大小块分配/回收，freelist 复用（7/18）
- [x] **Slab allocator** — 多尺寸 slab（8/16/32/64/128/256），roundUp 位运算对齐（7/19）
- [x] **Benchmark vs malloc** — 固定大小持平/更优，随机大小略慢（if-else 开销），毫秒级差距

### 第二阶段：多线程优化

- [x] **Thread-local cache** — 每个线程独立的 freelist，无锁分配。thread_local + static 实现（7/19）
- [ ] **Central cache** — 全局池，带自旋锁。线程间批量调剂内存，严格控制锁粒度
- [ ] **CAS 原子操作优化** — 用 `std::atomic` 的 `compare_exchange_weak` 实现无锁 freelist pop/push

### 第三阶段：工程化

- [ ] **内存对齐** — 支持自定义对齐字节，SIMD 必需
- [ ] **Debug 模式** — 检测 double-free、内存泄漏、越界写
- [ ] **Benchmark vs jemalloc** — 多线程混合大小分配性能对比

## Quick Start

```cpp
#include "fixed_allocator.h"

int main() {
    FixedAllocator<32> alloc;    // 管理 32 字节块

    void* p1 = alloc.allocate(); // 从 freelist 取一个块
    void* p2 = alloc.allocate();

    alloc.deallocate(p1);        // 放回 freelist
    alloc.deallocate(p2);
}
```

## Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

C++17，无外部依赖。

## License

MIT — 随便用，不追责就行。
