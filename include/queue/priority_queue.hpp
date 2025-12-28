#pragma once
#include "queue/bounded_queue.hpp"
#include "queue/unbounded_queue.hpp"
#include "types.hpp"

#include <atomic>
#include <condition_variable>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <unordered_map>

namespace dispatcher::queue {

class PriorityQueue {
    using Config = std::unordered_map<TaskPriority, QueueOptions>;

public:
    explicit PriorityQueue(const Config &config);

    void push(TaskPriority priority, Task task);
    // block on pop until shutdown is called
    // after that return std::nullopt on empty queue
    std::optional<Task> pop();

    void shutdown();

    ~PriorityQueue();

private:
    std::map<TaskPriority, std::unique_ptr<IQueue>> queues_;
    std::mutex mutex_;
    std::condition_variable not_empty_;
    bool active_ = true;
    std::atomic<size_t> size_ = 0;
};

}  // namespace dispatcher::queue