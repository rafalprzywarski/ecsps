#include <ecsps/ResourcePool.hpp>
#include <gtest/gtest.h>

namespace ecsps
{

struct ResourcePoolTest : testing::Test
{
    struct Resource
    {
        int id;
        Resource(int id) : id(id) { }
    };
    std::shared_ptr<ResourcePool<int, Resource>> pool = std::make_shared<ResourcePool<int, Resource>>([](int id) { return std::make_unique<Resource>(id); });
};

TEST_F(ResourcePoolTest, should_return_a_pointer_to_const_pointing_to_create_resource_for_a_given_id)
{
    std::shared_ptr<const Resource> res1 = pool->get(88);
    std::shared_ptr<const Resource> res2 = pool->get(101);
    ASSERT_EQ(88, res1->id);
    ASSERT_EQ(101, res2->id);
}

TEST_F(ResourcePoolTest, should_return_the_same_pointers_for_equal_ids)
{
    std::shared_ptr<const Resource> res = pool->get(88);
    ASSERT_TRUE(res == pool->get(88));
    ASSERT_TRUE(res == pool->get(88));
}

TEST_F(ResourcePoolTest, should_forget_resources_that_are_no_longer_referenced)
{
    std::shared_ptr<const Resource> res1 = pool->get(88);
    std::shared_ptr<const Resource> res2 = pool->get(99);
    std::weak_ptr<const Resource> ref1 = res1;
    std::weak_ptr<const Resource> ref2 = res2;

    res1.reset();

    ASSERT_TRUE(ref1.expired());
    ASSERT_FALSE(ref2.expired());
    ASSERT_TRUE(res2 == pool->get(99));
}

TEST_F(ResourcePoolTest, should_recreate_a_pointer_after_the_previos_resource_expired)
{
    std::shared_ptr<const Resource> res = pool->get(88);
    std::weak_ptr<const Resource> ref = res;
    res.reset();

    res = pool->get(88);
    ASSERT_EQ(88, res->id);
    ASSERT_TRUE(res == pool->get(88));
}

TEST_F(ResourcePoolTest, should_not_fail_when_pointers_expire_after_a_pool_is_destroyed)
{
    std::shared_ptr<const Resource> res = pool->get(88);
    std::weak_ptr<const Resource> ref = res;

    pool.reset();
    res.reset();
    ASSERT_TRUE(ref.expired());
}

}
