#pragma once
#include <cstddef>
#include <cstdint>

template <size_t BlockSize, size_t Align = 64>
class FixedAllocator {
    struct alignas(Align) Block {
        Block* next;
    };

public:
    FixedAllocator(size_t numBlocks = 1024) {
        size_t stride = (BlockSize + Align - 1) & ~(Align - 1);
        poolSize_ = stride * numBlocks + Align;
        originPool_ = new char[poolSize_];

        char* p = originPool_;
        uintptr_t addr = reinterpret_cast<uintptr_t>(p);
        addr = (addr + Align - 1) & ~(Align - 1);
        pool_ = reinterpret_cast<char*>(addr);

        for (size_t i = 0; i < numBlocks; i ++) {
            Block* b = reinterpret_cast<Block*>(pool_ + i * stride);
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
        return b;
    }

    void deallocate(void* p) {
        Block* b = static_cast<Block*>(p);
        b->next = freeList_;
        freeList_ = b;
    }

private:
    Block* freeList_ = nullptr;
    char* pool_ = nullptr;
    char* originPool_ = nullptr;
    size_t poolSize_;
};