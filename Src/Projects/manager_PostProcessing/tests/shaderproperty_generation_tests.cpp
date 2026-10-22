
/**    \file   shaderproperty_generation_tests.cpp

Sergei <Neill3d> Solokhin 2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

Tests for SceneGeneration (shaderproperty_generation.h), the counter that invalidates
FBProperties cached in property schemes after a scene change (file new / open, merge, delete).
*/

#include <gtest/gtest.h>

#include "shaderproperty_generation.h"

#include <cstdint>
#include <limits>

// a scheme that was never associated must never look current
TEST(SceneGenerationTest, NotAssociatedIsNeverCurrent)
{
    EXPECT_NE(SceneGeneration::Current(), SceneGeneration::NotAssociated);
    EXPECT_FALSE(SceneGeneration::IsCurrent(SceneGeneration::NotAssociated));

    SceneGeneration::Bump();
    EXPECT_NE(SceneGeneration::Current(), SceneGeneration::NotAssociated);
    EXPECT_FALSE(SceneGeneration::IsCurrent(SceneGeneration::NotAssociated));
}

// a scheme stamped before a scene change becomes stale, a re-stamped one is current again
TEST(SceneGenerationTest, BumpMakesStampStale)
{
    const uint32_t stamp = SceneGeneration::Current();
    EXPECT_TRUE(SceneGeneration::IsCurrent(stamp));

    SceneGeneration::Bump();
    EXPECT_FALSE(SceneGeneration::IsCurrent(stamp));

    const uint32_t newStamp = SceneGeneration::Current();
    EXPECT_NE(newStamp, stamp);
    EXPECT_TRUE(SceneGeneration::IsCurrent(newStamp));
}

// several scene changes in a row (file open, then merge) keep any older stamp stale
TEST(SceneGenerationTest, OlderStampsStayStale)
{
    const uint32_t first = SceneGeneration::Current();
    SceneGeneration::Bump();
    const uint32_t second = SceneGeneration::Current();
    SceneGeneration::Bump();

    EXPECT_FALSE(SceneGeneration::IsCurrent(first));
    EXPECT_FALSE(SceneGeneration::IsCurrent(second));
    EXPECT_TRUE(SceneGeneration::IsCurrent(SceneGeneration::Current()));
}

// a wrap around must not produce the NotAssociated value
TEST(SceneGenerationTest, WrapAroundSkipsNotAssociated)
{
    EXPECT_EQ(SceneGeneration::Next(1u), 2u);
    EXPECT_EQ(SceneGeneration::Next(std::numeric_limits<uint32_t>::max()), 1u);
    EXPECT_NE(SceneGeneration::Next(std::numeric_limits<uint32_t>::max()), SceneGeneration::NotAssociated);
}

// a file operation turns the gate on, its end bumps the generation, so panes collected during it are outdated
TEST(SceneGenerationTest, ChangeEndOutdatesStampsTakenDuringChange)
{
    EXPECT_FALSE(SceneGeneration::IsChanging());

    SceneGeneration::BeginChange();
    EXPECT_TRUE(SceneGeneration::IsChanging());

    // a render pane collected while the old scene is being destroyed
    const uint32_t stampDuringChange = SceneGeneration::Current();

    EXPECT_TRUE(SceneGeneration::EndChange());
    EXPECT_FALSE(SceneGeneration::IsChanging());
    EXPECT_FALSE(SceneGeneration::IsCurrent(stampDuringChange));
}

// an end without a begin (an idle after a completed operation) changes nothing
TEST(SceneGenerationTest, EndWithoutChangeKeepsGeneration)
{
    const uint32_t stamp = SceneGeneration::Current();
    EXPECT_FALSE(SceneGeneration::EndChange());
    EXPECT_TRUE(SceneGeneration::IsCurrent(stamp));
}
