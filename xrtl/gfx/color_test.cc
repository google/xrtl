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

#include "xrtl/gfx/color.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace gfx {
namespace {

TEST(ColorTest, ToUint32) {
  EXPECT_EQ(0x00FFFFFF, rgba8_t(0xFF, 0xFF, 0xFF, 0));
  EXPECT_EQ(0x99FFFFFF, rgba8_t(0xFF, 0xFF, 0xFF, 0x99));
  EXPECT_EQ(0xFF000000, rgba8_t(0, 0, 0, 0xFF));
  EXPECT_EQ(0xFFFFFFFF, rgba8_t(0xFF, 0xFF, 0xFF, 0xFF));
  EXPECT_EQ(0xFF999999, rgba8_t(0x99, 0x99, 0x99, 0xFF));
  EXPECT_EQ(0xFF0000FF, rgba8_t(0xFF, 0, 0, 0xFF));
  EXPECT_EQ(0xFF00FF00, rgba8_t(0, 0xFF, 0, 0xFF));
  EXPECT_EQ(0xFFFF0000, rgba8_t(0, 0, 0xFF, 0xFF));
  EXPECT_EQ(0xFF00FFFF, rgba8_t(0xFF, 0xFF, 0, 0xFF));
  EXPECT_EQ(0xFFFF00FF, rgba8_t(0xFF, 0, 0xFF, 0xFF));
  EXPECT_EQ(0xFFFFFF00, rgba8_t(0, 0xFF, 0xFF, 0xFF));
}

TEST(ColorTest, FromUint32) {
  EXPECT_EQ(rgba8_t(0, 0, 0, 0), rgba8_t(0x00000000));
  EXPECT_EQ(rgba8_t(0xFF, 0xFF, 0xFF, 0), rgba8_t(0x00FFFFFF));
  EXPECT_EQ(rgba8_t(0x99, 0x99, 0x99, 0), rgba8_t(0x00999999));
  EXPECT_EQ(rgba8_t(0xFF, 0, 0, 0), rgba8_t(0x000000FF));
  EXPECT_EQ(rgba8_t(0, 0xFF, 0, 0), rgba8_t(0x0000FF00));
  EXPECT_EQ(rgba8_t(0, 0, 0xFF, 0), rgba8_t(0x00FF0000));
  EXPECT_EQ(rgba8_t(0xFF, 0xFF, 0, 0), rgba8_t(0x0000FFFF));
  EXPECT_EQ(rgba8_t(0xFF, 0, 0xFF, 0), rgba8_t(0x00FF00FF));
  EXPECT_EQ(rgba8_t(0xFF, 0xFF, 0xFF, 0x00), rgba8_t(0x00FFFFFF));
  EXPECT_EQ(rgba8_t(0xFF, 0xFF, 0xFF, 0x99), rgba8_t(0x99FFFFFF));
  EXPECT_EQ(rgba8_t(0xFF, 0xFF, 0xFF, 0xFF), rgba8_t(0xFFFFFFFF));
}

TEST(ColorTest, IsTransparent) {
  EXPECT_FALSE(rgba8_t(0xFF, 0, 0, 0).is_opaque());
  EXPECT_FALSE(rgba8_t(0xFF, 0, 0, 0x99).is_opaque());
  EXPECT_TRUE(rgba8_t(0xFF, 0, 0, 0xFF).is_opaque());
  EXPECT_TRUE(rgba8_t(0xFF, 0, 0, 0).is_transparent());
  EXPECT_FALSE(rgba8_t(0xFF, 0, 0, 0x99).is_transparent());
  EXPECT_FALSE(rgba8_t(0xFF, 0, 0, 0xFF).is_transparent());
}

TEST(ColorTest, GetR) {
  EXPECT_EQ(0, rgba8_t(0, 0xFF, 0xFF, 0xFF).r);
  EXPECT_EQ(0x11, rgba8_t(0x11, 0xFF, 0xFF, 0xFF).r);
  EXPECT_EQ(0xFF, rgba8_t(0xFF, 0xFF, 0xFF, 0xFF).r);
}

TEST(ColorTest, GetG) {
  EXPECT_EQ(0, rgba8_t(0xFF, 0, 0xFF, 0xFF).g);
  EXPECT_EQ(0x11, rgba8_t(0xFF, 0x11, 0xFF, 0xFF).g);
  EXPECT_EQ(0xFF, rgba8_t(0xFF, 0xFF, 0xFF, 0xFF).g);
}

TEST(ColorTest, GetB) {
  EXPECT_EQ(0, rgba8_t(0xFF, 0xFF, 0, 0xFF).b);
  EXPECT_EQ(0x11, rgba8_t(0xFF, 0xFF, 0x11, 0xFF).b);
  EXPECT_EQ(0xFF, rgba8_t(0xFF, 0xFF, 0xFF, 0xFF).b);
}

TEST(ColorTest, GetA) {
  EXPECT_EQ(0, rgba8_t(0xFF, 0xFF, 0xFF, 0).a);
  EXPECT_EQ(0x11, rgba8_t(0xFF, 0xFF, 0xFF, 0x11).a);
  EXPECT_EQ(0xFF, rgba8_t(0xFF, 0xFF, 0xFF, 0xFF).a);
}

TEST(ColorTest, FromString) {
  EXPECT_EQ(0x00000000, color::FromString("00000000"));
  EXPECT_EQ(0xFFFFFFFF, color::FromString("FFFFFFFF"));
  EXPECT_EQ(0xFF000000, color::FromString("000000FF"));
  EXPECT_EQ(0x880000FF, color::FromString("FF000088"));
  EXPECT_EQ(0x8800FF00, color::FromString("00FF0088"));
  EXPECT_EQ(0x88FF0000, color::FromString("0000FF88"));
  EXPECT_EQ(0xFF000000, color::FromString("000000"));
  EXPECT_EQ(0xFFFFFFFF, color::FromString("FFFFFF"));
  EXPECT_EQ(0xFF0000FF, color::FromString("FF0000"));
  EXPECT_EQ(0xFF00FF00, color::FromString("00FF00"));
  EXPECT_EQ(0xFFFF0000, color::FromString("0000FF"));
  EXPECT_EQ(0xFF221100, color::FromString("#001122"));
  EXPECT_EQ(0x33221100, color::FromString("#00112233"));
  EXPECT_EQ(0x00000000, color::FromString(""));
  EXPECT_EQ(0x00000000, color::FromString("FF"));
  EXPECT_EQ(0x00000000, color::FromString("z"));
  EXPECT_EQ(0x00000000, color::FromString("zxdzf"));
}

TEST(ColorTest, Lerp) {
  EXPECT_EQ(rgba8_t(0xFD, 0x00, 0xFF, 0xFF),
            color::Lerp(rgba8_t(0xFF, 0x00, 0x00, 0xFF),
                        rgba8_t(0x00, 0x00, 0xFF, 0xFF),
                        absl::bit_cast<float>(0x3F008081)));
}

}  // namespace
}  // namespace gfx
}  // namespace xrtl
