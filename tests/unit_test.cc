
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <MyProject/example.hpp>

TEST(AddTest, SimpleAddition) {
    EXPECT_EQ(MyProject::add(1, 2), 3);
}
