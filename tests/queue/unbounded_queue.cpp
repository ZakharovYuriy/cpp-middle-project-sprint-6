#include <gtest/gtest.h>

#include <functional>
#include <thread>
#include <atomic>
#include <vector>

#include "queue/unbounded_queue.hpp"

using dispatcher::queue::UnboundedQueue;

TEST(UnboundedQueueTest, TryPopEmptyReturnsNullopt) {
    UnboundedQueue queue;
    EXPECT_FALSE(queue.try_pop().has_value());
}

TEST(UnboundedQueueTest, PushPopPreservesOrder) {
    UnboundedQueue queue;
    std::vector<int> order;

    queue.push([&order] { order.push_back(1); });
    queue.push([&order] { order.push_back(2); });
    queue.push([&order] { order.push_back(3); });

    auto first = queue.try_pop();
    ASSERT_TRUE(first.has_value());
    std::invoke(first.value());

    auto second = queue.try_pop();
    ASSERT_TRUE(second.has_value());
    std::invoke(second.value());

    auto third = queue.try_pop();
    ASSERT_TRUE(third.has_value());
    std::invoke(third.value());

    EXPECT_EQ(order, (std::vector<int>{1, 2, 3}));
    EXPECT_FALSE(queue.try_pop().has_value());
}

TEST(UnboundedQueueTest, PushAfterEmptyWorks) {
    UnboundedQueue queue;
    std::vector<int> order;

    EXPECT_FALSE(queue.try_pop().has_value());

    queue.push([&order] { order.push_back(1); });
    queue.push([&order] { order.push_back(2); });

    auto first = queue.try_pop();
    ASSERT_TRUE(first.has_value());
    std::invoke(first.value());

    auto second = queue.try_pop();
    ASSERT_TRUE(second.has_value());
    std::invoke(second.value());

    EXPECT_EQ(order, (std::vector<int>{1, 2}));
    EXPECT_FALSE(queue.try_pop().has_value());
}

TEST(UnboundedQueueTest, SupportsConcurrentPushes) {
    UnboundedQueue queue;
    std::atomic<int> executed{0};
    constexpr int kThreads = 4;
    constexpr int kTasksPerThread = 50;

    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j < kTasksPerThread; ++j) {
                queue.push([&executed] { executed.fetch_add(1, std::memory_order_relaxed); });
            }
        });
    }

    for (auto &thread : threads) {
        thread.join();
    }

    int popped = 0;
    while (auto task = queue.try_pop()) {
        std::invoke(task.value());
        ++popped;
    }

    EXPECT_EQ(popped, kThreads * kTasksPerThread);
    EXPECT_EQ(executed.load(std::memory_order_relaxed), kThreads * kTasksPerThread);
}

TEST(UnboundedQueueTest, TryPopRemovesElements) {
    UnboundedQueue queue;
    int value = 0;

    queue.push([&value] { value = 7; });

    auto task = queue.try_pop();
    ASSERT_TRUE(task.has_value());
    std::invoke(task.value());

    EXPECT_EQ(value, 7);
    EXPECT_FALSE(queue.try_pop().has_value());
}
