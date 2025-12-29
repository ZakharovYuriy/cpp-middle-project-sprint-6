#include "task_dispatcher.hpp"
#include <memory>

namespace dispatcher {
TaskDispatcher::TaskDispatcher(size_t thread_count, queue::Config config)
    : threadPool_(std::make_shared<queue::PriorityQueue>(config), thread_count) {}

void TaskDispatcher::schedule(TaskPriority priority, std::function<void()> task) { threadPool_.push(priority, task); }

}  // namespace dispatcher