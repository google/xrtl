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

#include "xrtl/base/intrusive_pool.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace {

int total_my_items_ctor_ = 0;
int total_my_items_dtor_ = 0;

struct MyItem {
  MyItem() { ++total_my_items_ctor_; }
  ~MyItem() { ++total_my_items_dtor_; }

  IntrusiveListLink link;
};

TEST(IntrusivePoolTest, Constructors) {
  IntrusivePool<MyItem> pool_a{0, 0};
  IntrusivePool<MyItem> pool_b{0, 100};
  IntrusivePool<MyItem> pool_c{100, 100};
}

TEST(IntrusivePoolTest, Disabled) {
  total_my_items_ctor_ = 0;
  total_my_items_dtor_ = 0;
  IntrusivePool<MyItem> pool{0, 0};
  auto item_a = pool.Allocate();
  auto item_b = pool.Allocate();
  auto item_c = pool.Allocate();
  EXPECT_EQ(3, total_my_items_ctor_);
  EXPECT_EQ(0, total_my_items_dtor_);
  pool.Release(item_a);
  pool.Release(item_b);
  pool.Release(item_c);
  EXPECT_EQ(3, total_my_items_ctor_);
  EXPECT_EQ(3, total_my_items_dtor_);
  auto item_d = pool.Allocate();
  EXPECT_EQ(4, total_my_items_ctor_);
  EXPECT_EQ(3, total_my_items_dtor_);
  pool.Release(item_d);
  EXPECT_EQ(4, total_my_items_ctor_);
  EXPECT_EQ(4, total_my_items_dtor_);
}

TEST(IntrusivePoolTest, OverflowPool) {
  total_my_items_ctor_ = 0;
  total_my_items_dtor_ = 0;
  IntrusivePool<MyItem> pool{0, 1};
  auto item_a = pool.Allocate();
  auto item_b = pool.Allocate();
  auto item_c = pool.Allocate();
  EXPECT_EQ(3, total_my_items_ctor_);
  EXPECT_EQ(0, total_my_items_dtor_);
  pool.Release(item_a);
  pool.Release(item_b);
  pool.Release(item_c);
  EXPECT_EQ(3, total_my_items_ctor_);
  EXPECT_EQ(3, total_my_items_dtor_);
  auto item_d = pool.Allocate();
  EXPECT_EQ(4, total_my_items_ctor_);
  EXPECT_EQ(3, total_my_items_dtor_);
  pool.Release(item_d);
  EXPECT_EQ(4, total_my_items_ctor_);
  EXPECT_EQ(4, total_my_items_dtor_);
}

TEST(IntrusivePoolTest, EntirelyPooled) {
  total_my_items_ctor_ = 0;
  total_my_items_dtor_ = 0;
  IntrusivePool<MyItem> pool{8, 8};
  auto item_a = pool.Allocate();
  auto item_b = pool.Allocate();
  auto item_c = pool.Allocate();
  EXPECT_EQ(3, total_my_items_ctor_);
  EXPECT_EQ(0, total_my_items_dtor_);
  pool.Release(item_a);
  pool.Release(item_b);
  pool.Release(item_c);
  EXPECT_EQ(3, total_my_items_ctor_);
  EXPECT_EQ(3, total_my_items_dtor_);
  auto item_d = pool.Allocate();
  EXPECT_EQ(4, total_my_items_ctor_);
  EXPECT_EQ(3, total_my_items_dtor_);
  pool.Release(item_d);
  EXPECT_EQ(4, total_my_items_ctor_);
  EXPECT_EQ(4, total_my_items_dtor_);
}

}  // namespace
}  // namespace xrtl
