#include "Cyclops/Tools/BrushEngine.h"

#include <gtest/gtest.h>
#include <glm/glm.hpp>

// Test Case 1: No Movement
// If we don't move, we should get 0 interpolation points.
TEST(BrushMath, NoMovementReturnsZeroPoints) {
    glm::vec2 start = { 0.0f, 0.0f };
    glm::vec2 end = { 0.0f, 0.0f };
    float spacing = 0.1f;
    float size = 10.0f;

    auto points = Cyclops::BrushEngine::CalculateInterpolation(start, end, spacing, size);

    EXPECT_EQ(points.size(), 0);
}

// Test Case 2: Linear Movement
// If spacing is 0.1 (10% of 10.0 size) and we move 10.0 units, we expect 10 points.
TEST(BrushMath, CorrectPointCount) {
    glm::vec2 start = { 0.0f, 0.0f };
    glm::vec2 end = { 10.0f, 0.0f };

    float size = 10.0f;
    float spacing = 0.1f; // Step size = 1.0f

    auto points = Cyclops::BrushEngine::CalculateInterpolation(start, end, spacing, size);

    // We expect exactly 10 points
    EXPECT_EQ(points.size(), 10);

    // Verify the last point is exactly at the end
    if (!points.empty()) {
        EXPECT_FLOAT_EQ(points.back().x, 10.0f);
        EXPECT_FLOAT_EQ(points.back().y, 0.0f);
    }
}

// Test Case 3: Large Spacing
// If spacing is 0.5 (50%), we expect fewer dots (gaps).
TEST(BrushMath, LargeSpacing) {
    glm::vec2 start = { 0.0f, 0.0f };
    glm::vec2 end = { 10.0f, 0.0f };

    float size = 10.0f;
    float spacing = 0.5f; // Step size = 5.0f

    auto points = Cyclops::BrushEngine::CalculateInterpolation(start, end, spacing, size);

    // Moving 10 units with 5 unit steps = 2 points (at 5.0 and 10.0)
    EXPECT_EQ(points.size(), 2);
}