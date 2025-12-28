#pragma once
#include "queue/priority_queue.hpp"
#include <memory>
#include <thread>
#include <vector>

namespace dispatcher::thread_pool {

class ThreadPool {
public:
    ThreadPool(std::shared_ptr<queue::PriorityQueue> priorityQueue,
               size_t threads = std::thread::hardware_concurrency());

    void push(TaskPriority priority, queue::Task task);

    ~ThreadPool();

private:
    void worker();

private:
    std::shared_ptr<queue::PriorityQueue> priorityQueue_;
    std::vector<std::jthread> workers_;
};

}  // namespace dispatcher::thread_pool
