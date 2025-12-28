#include "queue/priority_queue.hpp"

namespace dispatcher::queue {
PriorityQueue::PriorityQueue(const Config &config) {
    for (const auto &[priority, options] : config) {
        if (options.bounded) {
            if (!options.capacity || *options.capacity <= 0) {
                throw std::invalid_argument("Bounded queue requires positive capacity");
            }
            queues_.emplace(priority, std::make_unique<BoundedQueue>(*options.capacity));
        } else {
            queues_.emplace(priority, std::make_unique<UnboundedQueue>());
        }
    }

    if (!queues_.contains(TaskPriority::High) || !queues_.contains(TaskPriority::Normal)) {
        throw std::invalid_argument("Config must contain High and Normal priorities");
    }
}

void PriorityQueue::push(TaskPriority priority, Task task) {
    // Блокируем доступ к очереди, используя std::unique_lock
    std::unique_lock lock(mutex_);

    if (!active_)
        return;

    queues_.at(priority)->push(std::move(task));
    ++size_;

    //
    // После добавления элемента в очередь мы можем уведомить потребителей (которые ожидают внутри метода Pop),
    // о том, что в очереди появились новые данные, которые можно обработать
    //
    not_empty_.notify_one();
}

std::optional<Task> PriorityQueue::pop() {
    std::unique_lock lock(mutex_);

    //
    // Перед тем как заснуть, метод wait проверит условие, записанное в лямбда-функции
    //     - Если очередь не пустая ИЛИ она уже не активна, то выполнение продолжится без ожидания
    //     - Если очередь пустая И она ещё активна, то поток уснёт, сняв блокировку с мьютекса
    //
    not_empty_.wait(lock, [this] { return size_ != 0 || !active_; });

    if (size_ == 0)
        return std::nullopt;

    //
    // Наша очередь в случае деактивации позволяет обработать находящиеся внутри неё данные,
    // поэтому после wait мы не проверяем флаг active_
    //
    auto result = std::move(queues_.at(TaskPriority::High)->try_pop());
    if (result.has_value()) {
        --size_;
        return result.value();
    }
    --size_;
    return queues_.at(TaskPriority::Normal)->try_pop();
}

void PriorityQueue::shutdown() {
    std::lock_guard lock(mutex_);
    active_ = false;
    not_empty_.notify_all();
}

}  // namespace dispatcher::queue