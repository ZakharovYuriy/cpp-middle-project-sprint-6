#include "queue/unbounded_queue.hpp"

#include <mutex>
#include <queue>

namespace dispatcher::queue {

bool UnboundedQueue::push(Task task) {
    // Блокируем доступ к очереди для других потоков
    std::lock_guard lock(mutex_);

    // Добавляем элемент в очередь
    data_.push(std::move(task));

    // Перед выходом из метода вызовется деструктор объекта lock, и блокировка доступа к очереди будет снята
    return true;
}

std::optional<Task> UnboundedQueue::try_pop() {
    // Блокируем доступ к очереди для других потоков
    std::lock_guard lock(mutex_);

    // Если очередь пуста, нам нечего возвращать
    if (data_.empty()) {
        return std::nullopt;
    }

    // Извлекаем элемент из очереди
    // Поскольку мы выполняем эту операцию внутри контейнера, используя std::lock_guard для синхронизации,
    // мы гарантируем, что ни один поток, кроме нашего, не сможет одновременно с нами получить доступ к очереди
    Task data = std::move(data_.front());
    data_.pop();

    // Перед выходом из метода вызовется деструктор объекта lock, и блокировка доступа к очереди будет снята
    return data;
}

UnboundedQueue::~UnboundedQueue() = default;

}  // namespace dispatcher::queue
