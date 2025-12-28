#pragma once

#include <memory>

#include "queue/priority_queue.hpp"
#include "queue/queue.hpp"
#include "thread_pool/thread_pool.hpp"
#include "types.hpp"

namespace dispatcher {
inline const queue::Config &defaultConfig() {
    static const queue::Config cfg = [] {
        queue::Config c;
        c.emplace(TaskPriority::High, queue::QueueOptions{.bounded = true, .capacity = 1000});
        c.emplace(TaskPriority::Normal, queue::QueueOptions{.bounded = false});
        return c;
    }();
    return cfg;
}

class TaskDispatcher {
public:
    TaskDispatcher(size_t thread_count, queue::Config config = defaultConfig());

    void schedule(TaskPriority priority, std::function<void()> task);
    ~TaskDispatcher() = default;

private:
    thread_pool::ThreadPool threadPool_;
};

}  // namespace dispatcher