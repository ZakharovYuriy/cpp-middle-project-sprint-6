#include <gtest/gtest.h>

#include <functional>
#include <vector>

#include "queue/bounded_queue.hpp"

using dispatcher::queue::BoundedQueue;

TEST(BoundedQueueTest, TryPopEmptyReturnsNullopt) {
    BoundedQueue queue(3);
    EXPECT_FALSE(queue.try_pop().has_value());
}

TEST(BoundedQueueTest, PushPopPreservesOrder) {
    BoundedQueue queue(4);
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
}

TEST(BoundedQueueTest, HonorsCapacityAndDropsExtra) {
    BoundedQueue queue(3);
    std::vector<int> order;

    queue.push([&order] { order.push_back(1); });
    queue.push([&order] { order.push_back(2); });
    queue.push([&order] { order.push_back(3); });
    queue.push([&order] { order.push_back(4); });

    for (int i = 0; i < 3; ++i) {
        auto task = queue.try_pop();
        ASSERT_TRUE(task.has_value());
        std::invoke(task.value());
    }

    EXPECT_EQ(order, (std::vector<int>{1, 2, 3}));
    EXPECT_FALSE(queue.try_pop().has_value());
}
