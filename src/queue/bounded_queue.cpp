#include "queue/bounded_queue.hpp"

namespace dispatcher::queue {
BoundedQueue::BoundedQueue(int capacity) : data_(capacity), front_(capacity - 1) {}
void BoundedQueue::push(Task task) {
    const size_t front = front_.load(std::memory_order_acquire);
    size_t back = back_.load(std::memory_order_relaxed);
    if (back == front)
        return;  // Буфер полон

    data_[back] = std::forward<Task>(task);
    back_.store((back + 1) % data_.size(), std::memory_order_release);
}

std::optional<Task> BoundedQueue::try_pop() {
    const size_t back = back_.load(std::memory_order_acquire);
    const size_t front = (front_.load(std::memory_order_relaxed) + 1) % data_.size();
    if (front == back)
        return std::nullopt;  // Буфер пуст

    std::optional<Task> res(std::move(data_[front]));
    front_.store(front, std::memory_order_release);
    return res;
}
}  // namespace dispatcher::queue