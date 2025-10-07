
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <MyProject/example.hpp>

TEST(AddTest, SimpleAddition) {
	EXPECT_EQ(MyProject::add(1, 2), 3);
}
TEST(VolkTest, VolkInitialize) {
	EXPECT_EQ(MyProject::vk_init_test(), 0);
}
