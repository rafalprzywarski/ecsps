#include <ecsps/ValuePool.hpp>
#include <gtest/gtest.h>

namespace ecsps
{

struct ValuePoolTest : testing::Test
{
    std::shared_ptr<ValuePool<int>> pool = std::make_shared<ValuePool<int>>();
};

TEST_F(ValuePoolTest, should_return_a_pointer_to_const_pointing_to_a_given_value)
{
    std::shared_ptr<const int> val1 = pool->get(88);
    std::shared_ptr<const int> val2 = pool->get(101);
    ASSERT_EQ(88, *val1);
    ASSERT_EQ(101, *val2);
}

TEST_F(ValuePoolTest, should_return_the_same_pointers_for_equal_values)
{
    std::shared_ptr<const int> val = pool->get(88);
    ASSERT_TRUE(val == pool->get(88));
    ASSERT_TRUE(val == pool->get(88));
}

TEST_F(ValuePoolTest, should_forget_values_that_are_no_longer_referenced)
{
    std::shared_ptr<const int> val1 = pool->get(88);
    std::shared_ptr<const int> val2 = pool->get(99);
    std::weak_ptr<const int> ref1 = val1;
    std::weak_ptr<const int> ref2 = val2;

    val1.reset();

    ASSERT_TRUE(ref1.expired());
    ASSERT_FALSE(ref2.expired());
    ASSERT_TRUE(val2 == pool->get(99));
}

TEST_F(ValuePoolTest, should_recreate_a_pointer_after_the_previos_value_expired)
{
    std::shared_ptr<const int> val = pool->get(88);
    std::weak_ptr<const int> ref = val;
    val.reset();

    val = pool->get(88);
    ASSERT_EQ(88, *val);
    ASSERT_TRUE(val == pool->get(88));
}

TEST_F(ValuePoolTest, should_not_fail_when_pointers_expire_after_a_pool_is_destroyed)
{
    std::shared_ptr<const int> val = pool->get(88);
    std::weak_ptr<const int> ref = val;

    pool.reset();
    val.reset();
    ASSERT_TRUE(ref.expired());
}

}
