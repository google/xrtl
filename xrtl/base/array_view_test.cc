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

#include "xrtl/base/array_view.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace {

// Tests empty array views.
TEST(ArrayViewTest, Empty) {
  ArrayView<int> view;
  EXPECT_EQ(0, view.size());
  EXPECT_TRUE(view.empty());
  EXPECT_FALSE(view);
  EXPECT_TRUE(!view);
}

// Tests using explicit initialization with a data pointer and size.
// This also tests many of our generic operators.
TEST(ArrayViewTest, ExplicitData) {
  int list[3] = {0, 1, 2};
  ArrayView<int> view(list, 0);
  EXPECT_TRUE(view.empty());
  EXPECT_FALSE(view);
  EXPECT_TRUE(!view);
  EXPECT_EQ(nullptr, view.data());
  EXPECT_EQ(0, view.size());

  EXPECT_EQ(view.data(), view.begin());
  EXPECT_EQ(view.data() + view.size(), view.end());
  EXPECT_EQ(view.begin(), view.end());

  view = {list, 1};
  EXPECT_FALSE(view.empty());
  EXPECT_TRUE(view);
  EXPECT_FALSE(!view);
  EXPECT_EQ(list, view.data());
  EXPECT_EQ(1, view.size());
  EXPECT_EQ(0, view[0]);

  EXPECT_EQ(view.data(), view.begin());
  EXPECT_EQ(view.data() + view.size(), view.end());

  view = {list, 3};
  EXPECT_FALSE(view.empty());
  EXPECT_EQ(list, view.data());
  EXPECT_EQ(3, view.size());
  EXPECT_EQ(0, view[0]);
  EXPECT_EQ(1, view[1]);
  EXPECT_EQ(2, view[2]);

  EXPECT_EQ(view.data(), view.begin());
  EXPECT_EQ(view.data() + view.size(), view.end());
}

// Tests using C++ arrays (foo[]).
TEST(ArrayViewTest, CcArray) {
  int list_1[1] = {0};
  ArrayView<int> view = {list_1};
  EXPECT_FALSE(view.empty());
  EXPECT_TRUE(view);
  EXPECT_FALSE(!view);
  EXPECT_EQ(list_1, view.data());
  EXPECT_EQ(1, view.size());
  EXPECT_EQ(0, view[0]);

  int list_3[3] = {0, 1, 2};
  view = {list_3, 3};
  EXPECT_FALSE(view.empty());
  EXPECT_EQ(list_3, view.data());
  EXPECT_EQ(3, view.size());
  EXPECT_EQ(0, view[0]);
  EXPECT_EQ(1, view[1]);
  EXPECT_EQ(2, view[2]);
}

// Tests using std::arrays.
TEST(ArrayViewTest, StdArray) {
  std::array<int, 0> empty_list;
  ArrayView<int> view(empty_list);
  EXPECT_TRUE(view.empty());
  EXPECT_FALSE(view);
  EXPECT_TRUE(!view);
  EXPECT_EQ(nullptr, view.data());
  EXPECT_EQ(0, view.size());

  std::array<int, 1> list_1 = {{0}};
  view = {list_1};
  EXPECT_FALSE(view.empty());
  EXPECT_TRUE(view);
  EXPECT_FALSE(!view);
  EXPECT_EQ(list_1.data(), view.data());
  EXPECT_EQ(1, view.size());
  EXPECT_EQ(0, view[0]);

  std::array<int, 3> list_3 = {{0, 1, 2}};
  view = {list_3};
  EXPECT_FALSE(view.empty());
  EXPECT_EQ(3, view.size());
  EXPECT_EQ(list_3.data(), view.data());
  EXPECT_EQ(0, view[0]);
  EXPECT_EQ(1, view[1]);
  EXPECT_EQ(2, view[2]);
}

// Tests using std::initializer_list.
TEST(ArrayViewTest, StdInitializerList) {
  ArrayView<int> view({});
  EXPECT_TRUE(view.empty());
  EXPECT_FALSE(view);
  EXPECT_TRUE(!view);
  EXPECT_EQ(nullptr, view.data());
  EXPECT_EQ(0, view.size());

  view = {0};
  EXPECT_FALSE(view.empty());
  EXPECT_TRUE(view);
  EXPECT_FALSE(!view);
  EXPECT_EQ(1, view.size());
  EXPECT_EQ(0, view[0]);

  view = {{0, 1, 2}};
  EXPECT_FALSE(view.empty());
  EXPECT_EQ(3, view.size());
  EXPECT_EQ(0, view[0]);
  EXPECT_EQ(1, view[1]);
  EXPECT_EQ(2, view[2]);
}

// Tests using std::vector.
TEST(ArrayViewTest, StdVector) {
  std::vector<int> empty_list;
  ArrayView<int> view(empty_list);
  EXPECT_TRUE(view.empty());
  EXPECT_FALSE(view);
  EXPECT_TRUE(!view);
  EXPECT_EQ(nullptr, view.data());
  EXPECT_EQ(0, view.size());

  std::vector<int> list_1 = {0};
  view = ArrayView<int>{list_1};
  EXPECT_FALSE(view.empty());
  EXPECT_TRUE(view);
  EXPECT_FALSE(!view);
  EXPECT_EQ(list_1.data(), view.data());
  EXPECT_EQ(1, view.size());
  EXPECT_EQ(0, view[0]);

  std::vector<int> list_3 = {0, 1, 2};
  view = ArrayView<int>{list_3};
  EXPECT_FALSE(view.empty());
  EXPECT_EQ(3, view.size());
  EXPECT_EQ(list_3.data(), view.data());
  EXPECT_EQ(0, view[0]);
  EXPECT_EQ(1, view[1]);
  EXPECT_EQ(2, view[2]);
}

// Tests converting array views to vectors.
TEST(ArrayViewTest, ConvertToVector) {
  ArrayView<int> view({});
  EXPECT_TRUE(view.empty());
  std::vector<int> converted = {view.begin(), view.end()};
  EXPECT_TRUE(converted.empty());

  view = {0, 1, 2};
  EXPECT_EQ(3, view.size());
  EXPECT_EQ(0, view[0]);
  EXPECT_EQ(1, view[1]);
  EXPECT_EQ(2, view[2]);

  converted = {view.begin(), view.end()};
  EXPECT_EQ(3, converted.size());
  EXPECT_EQ(0, converted[0]);
  EXPECT_EQ(1, converted[1]);
  EXPECT_EQ(2, converted[2]);
}

}  // namespace
}  // namespace xrtl
