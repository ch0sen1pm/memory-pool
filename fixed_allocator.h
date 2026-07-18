#pragma once
#include <cstddef>

template <size_t BlockSize>
class FixedAllocator {
    struct Block {
        Block* next;
    };

public:
    FixedAllocator(size_t numBlocks = 1024) {
        poolSize_ = BlockSize * numBlocks;
        pool_ = new char[poolSize_];

        for (size_t i = 0; i < numBlocks; i ++) {
            Block* b = reinterpret_cast<Block*>(pool_ + i * BlockSize);
            b->next = freeList_;
            freeList_ = b;
        }
    }

    ~FixedAllocator() {
        delete[] pool_;
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
    size_t poolSize_;
};