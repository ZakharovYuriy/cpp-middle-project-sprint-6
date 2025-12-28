#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <mutex>
#include <stdexcept>
#include <vector>

#include "task_dispatcher.hpp"

using dispatcher::TaskDispatcher;
using dispatcher::TaskPriority;
using dispatcher::queue::Config;
using dispatcher::queue::QueueOptions;

namespace {

Config MakeUnboundedConfig() {
    Config config;
    config.emplace(TaskPriority::High, QueueOptions{.bounded = false});
    config.emplace(TaskPriority::Normal, QueueOptions{.bounded = false});
    return config;
}

bool WaitForCount(std::condition_variable &cv, std::mutex &mutex, const std::atomic<int> &count, int target,
                  std::chrono::milliseconds timeout) {
    std::unique_lock lock(mutex);
    return cv.wait_for(lock, timeout, [&count, target] { return count.load(std::memory_order_relaxed) == target; });
}

}  // namespace

TEST(TaskDispatcherTest, ExecutesScheduledTasks) {
    using namespace std::chrono_literals;

    TaskDispatcher dispatcher(2, MakeUnboundedConfig());
    std::atomic<int> completed{0};
    std::mutex mutex;
    std::condition_variable cv;

    for (int i = 0; i < 6; ++i) {
        dispatcher.schedule(TaskPriority::Normal, [&] {
            completed.fetch_add(1, std::memory_order_relaxed);
            cv.notify_one();
        });
    }

    EXPECT_TRUE(WaitForCount(cv, mutex, completed, 6, 1s));
}

TEST(TaskDispatcherTest, HighPriorityTasksRunBeforeNormalWhenQueued) {
    using namespace std::chrono_literals;

    TaskDispatcher dispatcher(1, MakeUnboundedConfig());
    std::atomic<int> completed{0};
    std::mutex mutex;
    std::condition_variable cv;
    std::vector<char> order;

    std::promise<void> gate_started;
    std::promise<void> gate_release;
    auto gate_started_future = gate_started.get_future();
    auto gate_release_future = gate_release.get_future().share();

    auto record = [&](char label) {
        {
            std::lock_guard lock(mutex);
            order.push_back(label);
        }
        completed.fetch_add(1, std::memory_order_relaxed);
        cv.notify_one();
    };

    dispatcher.schedule(TaskPriority::Normal, [&] {
        gate_started.set_value();
        gate_release_future.wait();
        record('G');
    });

    ASSERT_EQ(gate_started_future.wait_for(500ms), std::future_status::ready);

    dispatcher.schedule(TaskPriority::Normal, [&] { record('N'); });
    dispatcher.schedule(TaskPriority::High, [&] { record('H'); });
    dispatcher.schedule(TaskPriority::High, [&] { record('H'); });
    dispatcher.schedule(TaskPriority::Normal, [&] { record('N'); });

    gate_release.set_value();

    ASSERT_TRUE(WaitForCount(cv, mutex, completed, 5, 1s));

    std::vector<char> snapshot;
    {
        std::lock_guard lock(mutex);
        snapshot = order;
    }

    EXPECT_EQ(snapshot, (std::vector<char>{'G', 'H', 'H', 'N', 'N'}));
}

TEST(TaskDispatcherTest, ConstructorThrowsOnMissingPriorities) {
    Config config;
    config.emplace(TaskPriority::High, QueueOptions{.bounded = false});
    EXPECT_THROW(TaskDispatcher dispatcher(1, config), std::invalid_argument);
}

TEST(TaskDispatcherTest, ConstructorThrowsOnInvalidBoundedCapacity) {
    Config config;
    config.emplace(TaskPriority::High, QueueOptions{.bounded = true, .capacity = 0});
    config.emplace(TaskPriority::Normal, QueueOptions{.bounded = false});
    EXPECT_THROW(TaskDispatcher dispatcher(1, config), std::invalid_argument);
}
