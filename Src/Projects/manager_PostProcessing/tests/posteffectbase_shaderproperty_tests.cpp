#include <gtest/gtest.h>
#include "posteffectbase.h"

using namespace std;

TEST(ShaderPropertyTest, FlagOperations) {
    IEffectShaderConnections::ShaderProperty prop("Test", "uTest");

    // Initially no flags
    EXPECT_FALSE(prop.HasFlag(IEffectShaderConnections::PropertyFlag::SYSTEM));
    EXPECT_FALSE(prop.HasFlag(IEffectShaderConnections::PropertyFlag::IsClamped1));

    // Set SYSTEM flag
    prop.SetFlag(IEffectShaderConnections::PropertyFlag::SYSTEM, true);
    EXPECT_TRUE(prop.HasFlag(IEffectShaderConnections::PropertyFlag::SYSTEM));

    // Set IsClamped1 flag
    prop.SetFlag(IEffectShaderConnections::PropertyFlag::IsClamped1, true);
    EXPECT_TRUE(prop.HasFlag(IEffectShaderConnections::PropertyFlag::IsClamped1));

    // Unset SYSTEM flag
    prop.SetFlag(IEffectShaderConnections::PropertyFlag::SYSTEM, false);
    EXPECT_FALSE(prop.HasFlag(IEffectShaderConnections::PropertyFlag::SYSTEM));
    EXPECT_TRUE(prop.HasFlag(IEffectShaderConnections::PropertyFlag::IsClamped1));
}

TEST(ShaderPropertyTest, ValueVariant) {
    IEffectShaderConnections::ShaderProperty prop("Test", "uTest");

    // Default value is float[1] == 0.0f
    auto* f1 = std::get_if<std::array<float, 1>>(&prop.value);
    ASSERT_NE(f1, nullptr);
    EXPECT_FLOAT_EQ((*f1)[0], 0.0f);

    // Set int value
    //prop.SetValue(42);
    //auto* i = std::get_if<int>(&prop.value);
    //ASSERT_NE(i, nullptr);
    //EXPECT_EQ(*i, 42);

    // Set float value
    prop.SetValue(3.14f);
    f1 = std::get_if<std::array<float, 1>>(&prop.value);
    ASSERT_NE(f1, nullptr);
    EXPECT_FLOAT_EQ((*f1)[0], 3.14f);

    // Set double value (should store as float)
    prop.SetValue(2.718);
    f1 = std::get_if<std::array<float, 1>>(&prop.value);
    ASSERT_NE(f1, nullptr);
    EXPECT_FLOAT_EQ((*f1)[0], 2.718f);
}

// main() is provided by gtest, no need to define it here.