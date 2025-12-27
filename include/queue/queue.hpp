#pragma once
#include <functional>
#include <optional>

namespace dispatcher::queue {

struct QueueOptions {
    bool bounded;
    std::optional<int> capacity;
};

using Task = std::function<void()>;

class IQueue {
public:
    virtual ~IQueue() = default;
    virtual void push(Task task) = 0;
    virtual std::optional<Task> try_pop() = 0;
};

}  // namespace dispatcher::queue