#include <GraphRunner/Rhi/Rhi.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(RhiInstance, InstanceCreation) {
    EXPECT_NO_THROW({
        GraphRunner::Rhi::Instance("Rhi Test build", "GraphRunner", { }, { });
    });
}

TEST(RhiPhysicalDevice, PhysicalDeviceCreation) {
    auto i =
        GraphRunner::Rhi::Instance("Rhi Test build", "GraphRunner", { }, { });
    auto pdv = i.create_single_physical_device_with_best_vram( );
    ASSERT_TRUE(pdv.has_value( ));
}

TEST(RhiDevice, LogicalDeviceCreation) {
    auto i =
        GraphRunner::Rhi::Instance("Rhi Test build", "GraphRunner", { }, { });
    auto pdv = i.create_single_physical_device_with_best_vram( );
    auto dv = pdv.value( ).create_logical_device_with_single_graphic_queue({ });
    ASSERT_TRUE(dv.has_value( ));
}

TEST(RhiQueue, CommandQueueCreation) {
    auto i =
        GraphRunner::Rhi::Instance("Rhi Test build", "GraphRunner", { }, { });
    auto pdv = i.create_single_physical_device_with_best_vram( );
    auto dv = pdv.value( ).create_logical_device_with_single_graphic_queue({ });
    auto graphic_queue1 = dv->create_graphics_queue( );
    ASSERT_TRUE(graphic_queue1.has_value( ));
    auto graphic_queue2 = dv->create_graphics_queue( );
    ASSERT_FALSE(graphic_queue2.has_value( ));
}
