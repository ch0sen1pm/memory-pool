#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>

template <size_t BlockSize, size_t Align = 64, bool Debug = false>
class FixedAllocator {
    static constexpr uint64_t GUARD = 0xDEADBEEFCAFEBABE;

    struct alignas(Align) Block {
        Block* next;
    };

public:
    FixedAllocator(size_t numBlocks = 1024) {
        size_t stride = (BlockSize + Align - 1) & ~(Align - 1);
        if constexpr (Debug) {
            stride += 16;
        }
        poolSize_ = stride * numBlocks + Align;
        originPool_ = new char[poolSize_];

        char* p = originPool_;
        uintptr_t addr = reinterpret_cast<uintptr_t>(p);
        addr = (addr + Align - 1) & ~(Align - 1);
        pool_ = reinterpret_cast<char*>(addr);

        for (size_t i = 0; i < numBlocks; i ++) {
            Block* b = reinterpret_cast<Block*>(pool_ + i * stride);
            if constexpr (Debug) {
                // front guard 放在 next 后面（b+8），避免覆盖 next
                uint64_t* front = reinterpret_cast<uint64_t*>(
                    reinterpret_cast<char*>(b) + 8);
                *front = GUARD;
                uint64_t* back = reinterpret_cast<uint64_t*>(
                    reinterpret_cast<char*>(b) + stride - 8);
                *back = GUARD;
            }
            b->next = freeList_;
            freeList_ = b;
        }
    }

    ~FixedAllocator() {
        delete[] originPool_;
    }

    void* allocate() {
        if (!freeList_) {
            return nullptr;
        }
        Block* b = freeList_;
        freeList_ = b->next;
        if constexpr (Debug) {
            return reinterpret_cast<char*>(b) + 16;  // 跳过 next + front guard
        }
        return b;
    }

    void deallocate(void* p) {
        Block* b;
        if constexpr (Debug) {
            b = reinterpret_cast<Block*>(
                reinterpret_cast<char*>(p) - 16);    // 退回头（跳过 next + front guard）
            // front guard 在 next 后面（b+8）
            uint64_t* front = reinterpret_cast<uint64_t*>(
                reinterpret_cast<char*>(b) + 8);
            assert(*front == GUARD && "front guard corrupted!");
            size_t stride = ((BlockSize + Align - 1) & ~(Align - 1)) + 16;
            uint64_t* back = reinterpret_cast<uint64_t*>(
                reinterpret_cast<char*>(b) + stride - 8);
            assert(*back == GUARD && "back guard corrupted!");
        } else {
            b = static_cast<Block*>(p);
        }
        b->next = freeList_;
        freeList_ = b;
    }

private:
    Block* freeList_ = nullptr;
    char* pool_ = nullptr;
    char* originPool_ = nullptr;
    size_t poolSize_;
};