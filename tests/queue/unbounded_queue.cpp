#include <gtest/gtest.h>

#include <functional>
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
