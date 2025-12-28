#pragma once
#include "queue/queue.hpp"
#include <atomic>
#include <vector>

namespace dispatcher::queue {

class BoundedQueue : public IQueue {
public:
    explicit BoundedQueue(int capacity);

    bool push(Task task) override;

    std::optional<Task> try_pop() override;

    ~BoundedQueue() override;

private:
    std::vector<Task> data_;
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> back_{0};
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> front_{0};
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> size_{0};
};

}  // namespace dispatcher::queue
