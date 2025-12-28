#include "queue/bounded_queue.hpp"

namespace dispatcher::queue {
BoundedQueue::BoundedQueue(int capacity) : data_(capacity), front_(capacity - 1) {}
void BoundedQueue::push(Task task) {
    if (size_.load(std::memory_order_acquire) == data_.size())
        return;  // Буфер полон

    size_t back = back_.load(std::memory_order_relaxed);
    data_[back] = std::forward<Task>(task);
    back_.store((back + 1) % data_.size(), std::memory_order_release);
    size_.fetch_add(1, std::memory_order_release);
}

std::optional<Task> BoundedQueue::try_pop() {
    if (size_.load(std::memory_order_acquire) == 0)
        return std::nullopt;  // Буфер пуст

    const size_t front = (front_.load(std::memory_order_relaxed) + 1) % data_.size();
    std::optional<Task> res(std::move(data_[front]));
    front_.store(front, std::memory_order_release);
    size_.fetch_sub(1, std::memory_order_release);
    return res;
}

BoundedQueue::~BoundedQueue() = default;
}  // namespace dispatcher::queue
