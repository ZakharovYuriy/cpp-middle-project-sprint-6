#include "thread_pool/thread_pool.hpp"

namespace dispatcher::thread_pool {

ThreadPool::ThreadPool(std::shared_ptr<queue::PriorityQueue> priorityQueue, size_t threads)
    : priorityQueue_(priorityQueue) {
    workers_.reserve(threads);
    for (size_t i = 0; i < threads; ++i) {
        workers_.emplace_back(&ThreadPool::worker, this);
    }
}
void ThreadPool::push(TaskPriority priority, queue::Task task) { priorityQueue_->push(priority, std::move(task)); }

ThreadPool::~ThreadPool() {
    stop_ = true;
    priorityQueue_->shutdown();
}

void ThreadPool::worker() {
    while (!stop_) {
        auto task = priorityQueue_->pop();
        if (task.has_value()) {
            std::invoke(task.value());
        } else {
            std::this_thread::yield();
        }
    }
}

}  // namespace dispatcher::thread_pool