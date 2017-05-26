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

#include "xrtl/base/macros.h"

#include "xrtl/base/ref_ptr.h"
#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace {

TEST(MacrosTest, CountOf) {
  uint8_t a[1];
  EXPECT_EQ(1, count_of(a));
  uint8_t b[2];
  EXPECT_EQ(2, count_of(b));
  uint32_t c[2];
  EXPECT_EQ(2, count_of(c));
}

TEST(MacrosTest, MakeArray) {
  std::array<int, 1> a1 = make_array(1);
  EXPECT_EQ(1, a1.size());
  EXPECT_EQ(1, a1[0]);
  std::array<int, 2> a2 = make_array(1, 2);
  EXPECT_EQ(2, a2.size());
  EXPECT_EQ(1, a2[0]);
  EXPECT_EQ(2, a2[1]);
}

TEST(MacrosTest, BitCast) {
  EXPECT_EQ(0x3F9DF3B6, bit_cast<uint32_t>(1.234f));
  EXPECT_EQ(0x3FF3C0CA428C51F2ull, bit_cast<uint64_t>(1.234567890123));
  EXPECT_EQ(1.234f, bit_cast<float>(0x3F9DF3B6));
  EXPECT_EQ(1.234567890123, bit_cast<double>(0x3FF3C0CA428C51F2ull));
}

enum class MyBitmask : uint8_t {
  kNone = 0,
  kFoo = 0b01,
  kBar = 0b10,
  kAll = kFoo | kBar,
};
XRTL_BITMASK(MyBitmask);

TEST(MacrosTest, Bitmask) {
  MyBitmask value = MyBitmask::kNone;
  EXPECT_EQ(MyBitmask::kNone, value);

  // operator |
  EXPECT_EQ(0b01, static_cast<uint32_t>(MyBitmask::kFoo | MyBitmask::kNone));
  EXPECT_EQ(0b11, static_cast<uint32_t>(MyBitmask::kFoo | MyBitmask::kBar));
  EXPECT_EQ(0b11, static_cast<uint32_t>(MyBitmask::kFoo | MyBitmask::kAll));

  // operator |=
  value = MyBitmask::kFoo;
  value |= MyBitmask::kNone;
  EXPECT_EQ(0b01, static_cast<uint32_t>(value));
  value = MyBitmask::kFoo;
  value |= MyBitmask::kBar;
  EXPECT_EQ(0b11, static_cast<uint32_t>(value));
  value = MyBitmask::kFoo;
  value |= MyBitmask::kAll;
  EXPECT_EQ(0b11, static_cast<uint32_t>(value));

  // operator &
  EXPECT_EQ(0b00, static_cast<uint32_t>(MyBitmask::kFoo & MyBitmask::kBar));
  EXPECT_EQ(0b01, static_cast<uint32_t>(MyBitmask::kFoo & MyBitmask::kFoo));
  EXPECT_EQ(0b01, static_cast<uint32_t>(MyBitmask::kFoo & MyBitmask::kAll));

  // operator &=
  value = MyBitmask::kFoo;
  value &= MyBitmask::kBar;
  EXPECT_EQ(0b00, static_cast<uint32_t>(value));
  value = MyBitmask::kFoo;
  value &= MyBitmask::kFoo;
  EXPECT_EQ(0b01, static_cast<uint32_t>(value));
  value = MyBitmask::kFoo;
  value &= MyBitmask::kAll;
  EXPECT_EQ(0b01, static_cast<uint32_t>(value));

  // operator ^
  EXPECT_EQ(0b00, static_cast<uint32_t>(MyBitmask::kFoo ^ MyBitmask::kFoo));
  EXPECT_EQ(0b11, static_cast<uint32_t>(MyBitmask::kFoo ^ MyBitmask::kBar));
  EXPECT_EQ(0b10, static_cast<uint32_t>(MyBitmask::kFoo ^ MyBitmask::kAll));

  // operator ^=
  value = MyBitmask::kFoo;
  value ^= MyBitmask::kFoo;
  EXPECT_EQ(0b00, static_cast<uint32_t>(value));
  value = MyBitmask::kFoo;
  value ^= MyBitmask::kBar;
  EXPECT_EQ(0b11, static_cast<uint32_t>(value));
  value = MyBitmask::kFoo;
  value ^= MyBitmask::kAll;
  EXPECT_EQ(0b10, static_cast<uint32_t>(value));

  // operator ~
  EXPECT_EQ(0b11111110, static_cast<uint32_t>(~MyBitmask::kFoo));
  EXPECT_EQ(0b11111100, static_cast<uint32_t>(~MyBitmask::kAll));

  // any
  EXPECT_FALSE(any(MyBitmask::kNone));
  EXPECT_TRUE(any(MyBitmask::kFoo));
  EXPECT_TRUE(any(MyBitmask::kAll));
}

class MoveableType : public RefObject<MoveableType> {
 public:
  using RefObject::counter_;
};

TEST(MacrosTest, MoveToLambda) {
  auto moveable = make_ref<MoveableType>();
  EXPECT_EQ(1, moveable->counter_);
  auto moveable_baton = MoveToLambda(moveable);
  EXPECT_EQ(nullptr, moveable);
  auto lambda = [moveable_baton]() {
    auto moveable = std::move(moveable_baton.value);
    EXPECT_EQ(1, moveable->counter_);
  };
  lambda();
}

}  // namespace
}  // namespace xrtl
