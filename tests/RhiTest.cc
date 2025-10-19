#include <GraphRunner/Rhi/Rhi.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(RhiInstance, InstanceCreation) {
    EXPECT_NO_THROW({
        GraphRunner::Rhi::Instance("Rhi Test build", "GraphRunner", { }, { });
    });
}
