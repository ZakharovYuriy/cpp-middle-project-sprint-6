#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <functional>
#include <vector>

#include "queue/priority_queue.hpp"

using dispatcher::TaskPriority;
using dispatcher::queue::Config;
using dispatcher::queue::PriorityQueue;
using dispatcher::queue::QueueOptions;

namespace {

Config MakeDefaultConfig() {
    Config config;
    config.emplace(TaskPriority::High, QueueOptions{.bounded = false});
    config.emplace(TaskPriority::Normal, QueueOptions{.bounded = false});
    return config;
}

}  // namespace

TEST(PriorityQueueTest, ConstructorThrowsOnMissingPriorities) {
    Config config;
    config.emplace(TaskPriority::High, QueueOptions{.bounded = false});
    EXPECT_THROW(PriorityQueue queue(config), std::invalid_argument);
}

TEST(PriorityQueueTest, ConstructorThrowsOnInvalidBoundedCapacity) {
    Config config;
    config.emplace(TaskPriority::High, QueueOptions{.bounded = true, .capacity = 0});
    config.emplace(TaskPriority::Normal, QueueOptions{.bounded = false});
    EXPECT_THROW(PriorityQueue queue(config), std::invalid_argument);
}

TEST(PriorityQueueTest, PopPrefersHighPriorityTasks) {
    PriorityQueue queue(MakeDefaultConfig());
    std::vector<int> order;

    queue.push(TaskPriority::Normal, [&order] { order.push_back(1); });
    queue.push(TaskPriority::High, [&order] { order.push_back(2); });
    queue.push(TaskPriority::Normal, [&order] { order.push_back(3); });

    auto first = queue.pop();
    ASSERT_TRUE(first.has_value());
    std::invoke(first.value());

    auto second = queue.pop();
    ASSERT_TRUE(second.has_value());
    std::invoke(second.value());

    auto third = queue.pop();
    ASSERT_TRUE(third.has_value());
    std::invoke(third.value());

    EXPECT_EQ(order, (std::vector<int>{2, 1, 3}));
}

TEST(PriorityQueueTest, PopBlocksUntilTaskAvailable) {
    using namespace std::chrono_literals;

    PriorityQueue queue(MakeDefaultConfig());
    auto future = std::async(std::launch::async, [&queue] { return queue.pop(); });

    EXPECT_EQ(future.wait_for(50ms), std::future_status::timeout);

    int value = 0;
    queue.push(TaskPriority::High, [&value] { value = 42; });

    auto task = future.get();
    ASSERT_TRUE(task.has_value());
    std::invoke(task.value());
    EXPECT_EQ(value, 42);
}

TEST(PriorityQueueTest, ShutdownUnblocksPopAndRejectsPush) {
    using namespace std::chrono_literals;

    PriorityQueue queue(MakeDefaultConfig());
    auto future = std::async(std::launch::async, [&queue] { return queue.pop(); });

    EXPECT_EQ(future.wait_for(50ms), std::future_status::timeout);

    queue.shutdown();

    EXPECT_EQ(future.wait_for(200ms), std::future_status::ready);
    auto task = future.get();
    EXPECT_FALSE(task.has_value());

    int value = 0;
    queue.push(TaskPriority::High, [&value] { value = 7; });
    EXPECT_FALSE(queue.pop().has_value());
    EXPECT_EQ(value, 0);
}
