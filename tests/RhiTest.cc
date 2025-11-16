#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../src/Rhi/VRhi.h"

TEST(RhiInstance, InstanceCreation) {
    GraphRunner::Rhi::VInstance("Rhi Test build", "GraphRunner", { }, { });
    SUCCEED( );
}

TEST(RhiObjects, ObjectCreation) {
    auto i =
        GraphRunner::Rhi::VInstance("Rhi Test build", "GraphRunner", { }, { });
    auto pdv = i.create_single_physical_device_with_best_vram( );
    ASSERT_TRUE(pdv.has_value( ));
    auto dv = pdv.value( ).create_logical_device_with_single_graphic_queue({ });
    ASSERT_TRUE(dv.has_value( ));
    auto graphic_queue1 = dv->create_graphics_queue( );
    ASSERT_TRUE(graphic_queue1.has_value( ));
    auto graphic_queue2 = dv->create_graphics_queue( );
    ASSERT_FALSE(graphic_queue2.has_value( ));
}
