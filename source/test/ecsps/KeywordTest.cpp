#include <ecsps/Keyword.hpp>
#include <gtest/gtest.h>

namespace ecsps
{

TEST(KeywordTest, keywords_should_be_equal_iff_their_names_are_equal)
{
    Keyword a("xxx"), b("xxx"), c("yyy");
    EXPECT_TRUE(a == a);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_FALSE(b == c);
    EXPECT_TRUE(b != c);
    EXPECT_FALSE(a != b);
}

TEST(KeywordTest, should_provide_its_name_as_string)
{
    std::string name = Keyword("abcd").str();
    ASSERT_EQ("abcd", name);
}

TEST(KeywordTest, should_provide_std_hash)
{
    ASSERT_NE(0u, std::hash<Keyword>()(Keyword("abc")));
}

TEST(KeywordTest, should_provide_a_literal_operator)
{
    ASSERT_TRUE("word"_k == Keyword("word"));
}

}
