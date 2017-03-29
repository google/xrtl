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

#include "xrtl/base/math.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace math {
namespace {

TEST(MathTest, Sign) {
  EXPECT_EQ(-1, Sign(-1));
  EXPECT_EQ(1, Sign(1));
  EXPECT_EQ(1, Sign(0));

  EXPECT_EQ(1, Sign(123));
  EXPECT_EQ(1, Sign(123.4));
  EXPECT_EQ(1, Sign(123.4f));

  EXPECT_EQ(-1, Sign(-123));
  EXPECT_EQ(-1, Sign(-123.4));
  EXPECT_EQ(-1, Sign(-123.4f));
}

TEST(MathTest, Lerp) {
  EXPECT_EQ(0.0, Lerp(0.0, 1.0, 0.0));
  EXPECT_EQ(0.5, Lerp(0.0, 1.0, 0.5));
  EXPECT_EQ(1.0, Lerp(0.0, 1.0, 1.0));

  EXPECT_EQ(0.0, Lerp(0.0, 1.0, 0.0));
  EXPECT_EQ(-0.5, Lerp(0.0, -1.0, 0.5));
  EXPECT_EQ(-1.0, Lerp(0.0, -1.0, 1.0));

  EXPECT_EQ(5, Lerp(0, 10, 0.5));
  EXPECT_EQ(17, Lerp(10, 20, 0.7));

  EXPECT_EQ(-10, Lerp(-10, 30, 0.0));
  EXPECT_EQ(0, Lerp(-10, 30, 0.25));
  EXPECT_EQ(10, Lerp(-10, 30, 0.50));
  EXPECT_EQ(20, Lerp(-10, 30, 0.75));
  EXPECT_EQ(30, Lerp(-10, 30, 1.0));

  // t outside the suggested [0, 1] range works
  EXPECT_EQ(-1.0, Lerp(0.0, 1.0, -1.0));
  EXPECT_EQ(2.0, Lerp(0.0, 1.0, 2.0));
}

TEST(MathTest, InverseLerp) {
  EXPECT_EQ(0.0, InverseLerp(0.0, 1.0, 0.0));
  EXPECT_EQ(0.5, InverseLerp(0.0, 1.0, 0.5));
  EXPECT_EQ(1.0, InverseLerp(0.0, 1.0, 1.0));

  EXPECT_EQ(0.0, InverseLerp(0.0, -1.0, 0.0));
  EXPECT_EQ(0.5, InverseLerp(0.0, -1.0, -0.5));
  EXPECT_EQ(1.0, InverseLerp(0.0, -1.0, -1.0));

  EXPECT_EQ(0.5, InverseLerp(0.0, 10.0, 5.0));
  EXPECT_EQ(0.7, InverseLerp(10.0, 20.0, 17.0));

  EXPECT_EQ(0.0, InverseLerp(-10.0, 30.0, -10.0));
  EXPECT_EQ(0.25, InverseLerp(-10.0, 30.0, 0.0));
  EXPECT_EQ(0.50, InverseLerp(-10.0, 30.0, 10.0));
  EXPECT_EQ(0.75, InverseLerp(-10.0, 30.0, 20.0));
  EXPECT_EQ(1.0, InverseLerp(-10.0, 30.0, 30.0));

  // Values outside the range return t values outside [0, 1]
  EXPECT_EQ(-1.0, InverseLerp(0.0, 1.0, -1.0));
  EXPECT_EQ(2.0, InverseLerp(0.0, 1.0, 2.0));
}

TEST(MathTest, Constrain) {
  EXPECT_EQ(0.0, Constrain(0.0, 0.0, 1.0, 0.5));
  EXPECT_EQ(0.5, Constrain(0.5, 0.0, 1.0, 0.5));
  EXPECT_EQ(1.0, Constrain(1.0, 0.0, 1.0, 0.5));

  EXPECT_EQ(0.0, Constrain(0.0, -1.0, 0.0, 0.5));
  EXPECT_EQ(-0.5, Constrain(-0.5, -1.0, 0.0, 0.5));
  EXPECT_EQ(-1.0, Constrain(-1.0, -1.0, 0.0, 0.5));

  EXPECT_EQ(1.5, Constrain(2.0, 0.0, 1.0, 0.5));
  // Scale of 0.0 clamps.
  EXPECT_EQ(1.0, Constrain(2.0, 0.0, 1.0, 0.0));
  // Scale of 1.0 does not change.
  EXPECT_EQ(2.0, Constrain(2.0, 0.0, 1.0, 1.0));

  // Outside range difference is scaled by 0.5.
  EXPECT_EQ(-5.0, Constrain(-20.0, 10.0, 20.0, 0.5));
  EXPECT_EQ(0.0, Constrain(-10.0, 10.0, 20.0, 0.5));
  EXPECT_EQ(5.0, Constrain(0.0, 10.0, 20.0, 0.5));
  // Inside range is not changed.
  EXPECT_EQ(10.0, Constrain(10.0, 10.0, 20.0, 0.5));
  EXPECT_EQ(15.0, Constrain(15.0, 10.0, 20.0, 0.5));
  EXPECT_EQ(20.0, Constrain(20.0, 10.0, 20.0, 0.5));
  // Outside range difference is scaled by 0.5.
  EXPECT_EQ(25.0, Constrain(30.0, 10.0, 20.0, 0.5));
  EXPECT_EQ(30.0, Constrain(40.0, 10.0, 20.0, 0.5));
  EXPECT_EQ(35.0, Constrain(50.0, 10.0, 20.0, 0.5));
}

TEST(MathTest, Clamp) {
  EXPECT_EQ(0.0, Clamp(0.0, 0.0, 1.0));
  EXPECT_EQ(0.5, Clamp(0.5, 0.0, 1.0));
  EXPECT_EQ(1.0, Clamp(1.0, 0.0, 1.0));

  EXPECT_EQ(11.0, Clamp(53.0, -21.0, 11.0));
  EXPECT_EQ(11.5, Clamp(53.0, -21.0, 11.5));
  EXPECT_EQ(11.5, Clamp(11.6, -21.0, 11.5));
  EXPECT_EQ(-21.0, Clamp(-30.0, -21.0, 11.5));
}

TEST(MathTest, Wrap) {
  EXPECT_EQ(1, Wrap(-1, 2));
  EXPECT_EQ(0, Wrap(0, 2));
  EXPECT_EQ(1, Wrap(1, 2));
  EXPECT_EQ(0, Wrap(2, 2));
  EXPECT_EQ(1, Wrap(3, 2));
  EXPECT_EQ(0, Wrap(4, 2));

  EXPECT_EQ(0, Wrap(-4, 4));
  EXPECT_EQ(1, Wrap(-3, 4));
  EXPECT_EQ(2, Wrap(-2, 4));
  EXPECT_EQ(3, Wrap(-1, 4));
  EXPECT_EQ(0, Wrap(0, 4));
  EXPECT_EQ(1, Wrap(1, 4));
  EXPECT_EQ(2, Wrap(2, 4));
  EXPECT_EQ(3, Wrap(3, 4));
  EXPECT_EQ(0, Wrap(4, 4));
  EXPECT_EQ(1, Wrap(5, 4));
  EXPECT_EQ(2, Wrap(6, 4));
}

TEST(MathTest, RoundToAlignment) {
  EXPECT_EQ(0, RoundToAlignment(0, 1));
  EXPECT_EQ(0, RoundToAlignment(0, 2));
  EXPECT_EQ(2, RoundToAlignment(1, 2));
  EXPECT_EQ(2, RoundToAlignment(2, 2));
  EXPECT_EQ(16, RoundToAlignment(4, 16));
  EXPECT_EQ(16, RoundToAlignment(16, 16));
  EXPECT_EQ(32, RoundToAlignment(17, 16));
}

TEST(MathTest, RoundToNextPowerOf2) {
  EXPECT_EQ(0, RoundToNextPowerOfTwo(0));
  EXPECT_EQ(1, RoundToNextPowerOfTwo(1));
  EXPECT_EQ(2, RoundToNextPowerOfTwo(2));
  EXPECT_EQ(4, RoundToNextPowerOfTwo(3));
  EXPECT_EQ(4, RoundToNextPowerOfTwo(4));
  EXPECT_EQ(128, RoundToNextPowerOfTwo(100));
  EXPECT_EQ(1024, RoundToNextPowerOfTwo(1000));
  EXPECT_EQ(1024, RoundToNextPowerOfTwo(1024));
  EXPECT_EQ(2048, RoundToNextPowerOfTwo(1025));
}

}  // namespace
}  // namespace math
}  // namespace xrtl
