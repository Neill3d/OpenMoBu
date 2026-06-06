
/**    \file   posteffectbase_shaderproperty_tests.cpp

Sergei <Neill3d> Solokhin 2025-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

Tests for the shader_property_core static library:
    shaderproperty_value.h/.cpp  — ShaderPropertyValue
    shaderproperty_storage.h     — ShaderPropertyStorage

The test executable links against shader_property_core only — no fbsdk,
no MoBu plugin DLL.  Classes that depend on fbsdk (ShaderProperty,
ShaderPropertyScheme, ShaderPropertyWriter) are compiled into the plugin
DLL and are tested separately through integration tests.
*/

#include <gtest/gtest.h>

#include "shaderproperty_value.h"
#include "shaderproperty_storage.h"

// ---------------------------------------------------------------------------
// Test 1 — ShaderPropertyValue: type initialisation and SetValue round-trips
// ---------------------------------------------------------------------------
// GetFloatData() exposes the underlying std::variant as a raw float pointer.
// Verify that SetType initialises each variant alternative correctly and that
// every SetValue() overload stores and retrieves the right data.
TEST(ShaderPropertyValueTest, TypeAndValueRoundTrip)
{
    // --- FLOAT ---
    ShaderPropertyValue floatVal(EPropertyType::FLOAT);
    EXPECT_EQ(floatVal.GetType(), EPropertyType::FLOAT);
    ASSERT_NE(floatVal.GetFloatData(), nullptr);
    EXPECT_FLOAT_EQ(floatVal.GetFloatData()[0], 0.0f);     // zero-initialised

    floatVal.SetValue(3.14f);
    EXPECT_FLOAT_EQ(floatVal.GetFloatData()[0], 3.14f);

    // double overload must truncate to float without UB
    floatVal.SetValue(2.718);
    EXPECT_FLOAT_EQ(floatVal.GetFloatData()[0], 2.718f);

    // bool overload: true -> 1.0f, false -> 0.0f
    floatVal.SetValue(true);
    EXPECT_FLOAT_EQ(floatVal.GetFloatData()[0], 1.0f);
    floatVal.SetValue(false);
    EXPECT_FLOAT_EQ(floatVal.GetFloatData()[0], 0.0f);

    // --- VEC2 ---
    ShaderPropertyValue vec2Val(EPropertyType::VEC2);
    EXPECT_EQ(vec2Val.GetType(), EPropertyType::VEC2);
    vec2Val.SetValue(0.25f, 0.75f);
    const float* d2 = vec2Val.GetFloatData();
    ASSERT_NE(d2, nullptr);
    EXPECT_FLOAT_EQ(d2[0], 0.25f);
    EXPECT_FLOAT_EQ(d2[1], 0.75f);

    // --- VEC4 ---
    // SetType initialises w to 1.0 (see ShaderPropertyValue::SetType case VEC4)
    ShaderPropertyValue vec4Val(EPropertyType::VEC4);
    EXPECT_EQ(vec4Val.GetType(), EPropertyType::VEC4);
    const float* init4 = vec4Val.GetFloatData();
    ASSERT_NE(init4, nullptr);
    EXPECT_FLOAT_EQ(init4[3], 1.0f);   // documented VEC4 default for w

    vec4Val.SetValue(0.1f, 0.2f, 0.3f, 0.4f);
    const float* d4 = vec4Val.GetFloatData();
    EXPECT_FLOAT_EQ(d4[0], 0.1f);
    EXPECT_FLOAT_EQ(d4[1], 0.2f);
    EXPECT_FLOAT_EQ(d4[2], 0.3f);
    EXPECT_FLOAT_EQ(d4[3], 0.4f);

    // --- Metadata fields are independent of the stored data ---
    ShaderPropertyValue meta(EPropertyType::FLOAT);
    meta.SetValue(0.5f);
    meta.SetScale(2.0f);
    meta.SetInvertValue(true);
    EXPECT_FLOAT_EQ(meta.GetScale(), 2.0f);
    EXPECT_TRUE(meta.IsInvertValue());
    EXPECT_FLOAT_EQ(meta.GetFloatData()[0], 0.5f);  // value unaffected by scale/invert
}

// ---------------------------------------------------------------------------
// Test 2 — ShaderPropertyStorage: per-effect isolation across two shaders
// ---------------------------------------------------------------------------
// The EffectMap layer (unordered_map<uint32_t, PropertyValueMap>) must keep
// each effect's properties independent.  Writing to effect A must not bleed
// into effect B, and CommitWrite must expose both simultaneously on the read
// side while clearing the new write buffer for the next frame.
TEST(ShaderPropertyStorageTest, MultiEffectIsolation)
{
    ShaderPropertyStorage storage;

    constexpr uint32_t kEffectA  = 0xAAAA0001u;
    constexpr uint32_t kEffectB  = 0xBBBB0002u;
    constexpr uint32_t kPropA    = 0x00000AAu;
    constexpr uint32_t kPropB1   = 0x00000BB1u;
    constexpr uint32_t kPropB2   = 0x00000BB2u;

    // --- Populate two independent effect write-maps ---
    {
        ShaderPropertyValue a(EPropertyType::FLOAT);
        a.SetNameHash(kPropA);
        a.SetValue(1.0f);
        storage.GetWritePropertyMap(kEffectA).emplace_back(a);
    }
    {
        ShaderPropertyStorage::PropertyValueMap& bMap =
            storage.GetWritePropertyMap(kEffectB);

        ShaderPropertyValue b1(EPropertyType::VEC2);
        b1.SetNameHash(kPropB1);
        b1.SetValue(0.5f, 0.6f);
        bMap.emplace_back(b1);

        ShaderPropertyValue b2(EPropertyType::FLOAT);
        b2.SetNameHash(kPropB2);
        b2.SetValue(7.0f);
        bMap.emplace_back(b2);
    }

    // Before commit both read maps are empty
    EXPECT_EQ(storage.GetReadPropertyMap(kEffectA), nullptr);
    EXPECT_EQ(storage.GetReadPropertyMap(kEffectB), nullptr);

    storage.CommitWrite(1u);

    // --- Effect A: exactly one property ---
    const auto* readA = storage.GetReadPropertyMap(kEffectA);
    ASSERT_NE(readA, nullptr);
    ASSERT_EQ(readA->size(), 1u);
    EXPECT_EQ((*readA)[0].GetNameHash(), kPropA);
    EXPECT_FLOAT_EQ((*readA)[0].GetFloatData()[0], 1.0f);

    // Effect A must not contain effect B's properties
    EXPECT_FALSE(ShaderPropertyStorage::CheckPropertyExists(*readA, kPropB1));
    EXPECT_FALSE(ShaderPropertyStorage::CheckPropertyExists(*readA, kPropB2));

    // --- Effect B: exactly two properties, in write order ---
    const auto* readB = storage.GetReadPropertyMap(kEffectB);
    ASSERT_NE(readB, nullptr);
    ASSERT_EQ(readB->size(), 2u);
    EXPECT_EQ((*readB)[0].GetNameHash(), kPropB1);
    EXPECT_EQ((*readB)[1].GetNameHash(), kPropB2);
    EXPECT_FLOAT_EQ((*readB)[0].GetFloatData()[0], 0.5f);
    EXPECT_FLOAT_EQ((*readB)[1].GetFloatData()[0], 7.0f);

    // Effect B must not contain effect A's property
    EXPECT_FALSE(ShaderPropertyStorage::CheckPropertyExists(*readB, kPropA));

    // --- CommitWrite cleared the new write buffer for both effects ---
    EXPECT_EQ(storage.GetWritePropertyMap(kEffectA).size(), 0u);
    EXPECT_EQ(storage.GetWritePropertyMap(kEffectB).size(), 0u);
}

// ---------------------------------------------------------------------------
// Test 3 — ShaderPropertyStorage: double-buffer write / commit / read cycle
// ---------------------------------------------------------------------------
// Core lock-free guarantee: a value written to the write buffer is invisible
// on the read side until CommitWrite() atomically flips the index, and the
// former read buffer is cleared so the next eval frame starts empty.
// Also covers CheckPropertyExists() — the linear scan used by the disabled
// VERIFY in CollectUIValues.
TEST(ShaderPropertyStorageTest, DoubleBufferWriteCommitRead)
{
    ShaderPropertyStorage storage;

    constexpr uint32_t kEffectHash  = 0xABCD1234u;
    constexpr uint32_t kPropHash    = 0x11223344u;
    constexpr uint32_t kMissingHash = 0xDEADBEEFu;

    // --- Populate the write buffer ---
    ShaderPropertyStorage::PropertyValueMap& writeMap =
        storage.GetWritePropertyMap(kEffectHash);

    ShaderPropertyValue val(EPropertyType::FLOAT);
    val.SetNameHash(kPropHash);
    val.SetValue(1.5f);
    writeMap.emplace_back(val);
    EXPECT_EQ(writeMap.size(), 1u);

    // --- Before commit: read buffer is still empty ---
    EXPECT_EQ(storage.GetReadPropertyMap(kEffectHash), nullptr);

    // --- After CommitWrite: written data is visible on the read side ---
    storage.CommitWrite(1u);

    const ShaderPropertyStorage::PropertyValueMap* readMap =
        storage.GetReadPropertyMap(kEffectHash);
    ASSERT_NE(readMap, nullptr);
    ASSERT_EQ(readMap->size(), 1u);
    EXPECT_EQ((*readMap)[0].GetNameHash(), kPropHash);
    ASSERT_NE((*readMap)[0].GetFloatData(), nullptr);
    EXPECT_FLOAT_EQ((*readMap)[0].GetFloatData()[0], 1.5f);

    // --- CommitWrite cleared the former read buffer ---
    // GetWritePropertyMap now targets the ex-read buffer (cleared by CommitWrite).
    // operator[] on an empty EffectMap creates a fresh empty PropertyValueMap.
    const ShaderPropertyStorage::PropertyValueMap& freshWrite =
        storage.GetWritePropertyMap(kEffectHash);
    EXPECT_EQ(freshWrite.size(), 0u);

    // --- CheckPropertyExists: O(N) scan by nameHash ---
    EXPECT_TRUE(ShaderPropertyStorage::CheckPropertyExists(*readMap, kPropHash));
    EXPECT_FALSE(ShaderPropertyStorage::CheckPropertyExists(*readMap, kMissingHash));

    // Empty map always returns false
    ShaderPropertyStorage::PropertyValueMap emptyMap;
    EXPECT_FALSE(ShaderPropertyStorage::CheckPropertyExists(emptyMap, kPropHash));
}
