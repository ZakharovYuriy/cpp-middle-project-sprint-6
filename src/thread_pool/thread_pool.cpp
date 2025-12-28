#include "thread_pool/thread_pool.hpp"

#include <cstdio>
#include <exception>

namespace dispatcher::thread_pool {

ThreadPool::ThreadPool(std::shared_ptr<queue::PriorityQueue> priorityQueue, size_t threads)
    : priorityQueue_(priorityQueue) {
    workers_.reserve(threads);
    for (size_t i = 0; i < threads; ++i) {
        workers_.emplace_back(&ThreadPool::worker, this);
    }
}
void ThreadPool::push(TaskPriority priority, queue::Task task) { priorityQueue_->push(priority, std::move(task)); }

ThreadPool::~ThreadPool() { priorityQueue_->shutdown(); }

void ThreadPool::worker() {
    while (true) {
        auto task = priorityQueue_->pop();
        if (!task.has_value()) {
            return;
        }

        try {
            std::invoke(task.value());
        } catch (const std::exception &ex) {
            std::fprintf(stderr, "Task execution failed: %s\n", ex.what());
        } catch (...) {
            std::fprintf(stderr, "Task execution failed: unknown exception\n");
        }
    }
}

}  // namespace dispatcher::thread_pool
