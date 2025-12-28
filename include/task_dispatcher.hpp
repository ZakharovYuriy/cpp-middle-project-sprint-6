#pragma once

#include <memory>

#include "queue/priority_queue.hpp"
#include "thread_pool/thread_pool.hpp"
#include "types.hpp"

namespace dispatcher {
const queue::Config &defaultConfig() {
    static const queue::Config cfg = [] {
        queue::Config c;
        c.emplace(TaskPriority::High, std::make_unique<queue::BoundedQueue>(1000));
        c.emplace(TaskPriority::Normal, std::make_unique<queue::UnboundedQueue>());
        return c;
    }();
    return cfg;
}

class TaskDispatcher {
public:
    TaskDispatcher(size_t thread_count, queue::Config config = defaultConfig());

    void schedule(TaskPriority priority, std::function<void()> task);
    ~TaskDispatcher();
};

}  // namespace dispatcher