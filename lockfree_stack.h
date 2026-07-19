#pragma once
#include <atomic>

template <typename T>
class LockFreeStack {
public:
    struct Node {
        T data;
        Node* next;
    };

    void push(Node* node) {
        node->next = head_.load(std::memory_order_relaxed);

        while (!head_.compare_exchange_weak(
            node->next,
            node,
            std::memory_order_release,
            std::memory_order_relaxed
        )) {
        }
    }

    Node* pop() {
        Node* node = head_.load(std::memory_order_acquire);
        while (node && !head_.compare_exchange_weak(
            node,
            node->next,
            std::memory_order_acquire,
            std::memory_order_relaxed
        )) {
        }

        return node;
    }

private:
    std::atomic<Node*> head_{nullptr};
};