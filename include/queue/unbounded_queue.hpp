#pragma once
#include "queue/queue.hpp"
#include <mutex>
#include <queue>

namespace dispatcher::queue {

class UnboundedQueue : public IQueue {
public:
    explicit UnboundedQueue();

    void push(std::function<void()> task) override;

    std::optional<std::function<void()>> try_pop() override;

    ~UnboundedQueue() override;

private:
    std::queue<Task> data_;
    std::mutex mutex_;  // для синхронизации внутри очереди мы будем использовать мьютекс
    bool active_ = true;
};

}  // namespace dispatcher::queue