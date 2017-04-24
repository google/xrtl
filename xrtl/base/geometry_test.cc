// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "xrtl/base/geometry.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace {

TEST(GeometryTest, RectIntersection) {
  Rect2D rect = {1000, 2000, 100, 200};

  // Should intersect with self.
  EXPECT_TRUE(rect.TestIntersection(rect));

  // Other rect fully inside of this rect.
  EXPECT_TRUE(rect.TestIntersection({1025, 2025, 50, 50}));
  // Other rect fully enclosing this rect.
  EXPECT_TRUE(rect.TestIntersection({0, 0, 5000, 5000}));

  // Other rect far away from this rect.
  EXPECT_FALSE(rect.TestIntersection({3000, 4000, 50, 50}));

  // Intersect with each side.
  EXPECT_TRUE(rect.TestIntersection({950, 2025, 200, 50}));
  EXPECT_TRUE(rect.TestIntersection({1050, 2025, 200, 50}));
  EXPECT_TRUE(rect.TestIntersection({1025, 1950, 50, 200}));
  EXPECT_TRUE(rect.TestIntersection({1025, 2050, 50, 200}));

  // Other rect touching the edge of this rect, should intersect.
  EXPECT_TRUE(rect.TestIntersection({1100, 2050, 100, 200}));
  // Other rect barely not touching the edge of this rect, should not intersect.
  EXPECT_FALSE(rect.TestIntersection({1101, 2050, 100, 200}));

  // Negative origin.
  rect = {-1000, -2000, 100, 200};
  EXPECT_TRUE(rect.TestIntersection(rect));
  EXPECT_TRUE(rect.TestIntersection({-975, -1975, 50, 50}));
  EXPECT_FALSE(rect.TestIntersection({1000, 2000, 100, 200}));
}

}  // namespace
}  // namespace xrtl
