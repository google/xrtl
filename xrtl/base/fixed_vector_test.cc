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

#include "xrtl/base/fixed_vector.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace {

// Tests a zero-capacity fixed vector.
// Don't know why you'd do this, but it should work. Note that std::array does
// not compile at 0.
TEST(FixedVectorTest, ZeroCapacity) {
  FixedVector<int, 0> vector;
  EXPECT_TRUE(vector.empty());
  EXPECT_EQ(0, vector.size());
  EXPECT_EQ(0, vector.max_size());
  EXPECT_FALSE(vector.contains(0));
  EXPECT_FALSE(vector.contains(1));
  EXPECT_EQ(-1, vector.index_of(0));
  EXPECT_EQ(-1, vector.index_of(1));
  vector.clear();
  EXPECT_EQ(vector.begin(), vector.end());
}

// Tests the behavior of an empty fixed vector.
TEST(FixedVectorTest, Empty) {
  FixedVector<int, 2> vector;
  EXPECT_TRUE(vector.empty());
  EXPECT_EQ(0, vector.size());
  EXPECT_EQ(2, vector.max_size());
  EXPECT_FALSE(vector.contains(0));
  EXPECT_FALSE(vector.contains(1));
  EXPECT_EQ(-1, vector.index_of(0));
  EXPECT_EQ(-1, vector.index_of(1));
  vector.clear();
  EXPECT_EQ(vector.begin(), vector.end());
}

// Tests basic vector operation.
TEST(FixedVectorTest, Basic) {
  // Initial, vector = [].
  FixedVector<int, 2> vector;
  EXPECT_TRUE(vector.empty());
  EXPECT_EQ(0, vector.size());
  EXPECT_EQ(2, vector.max_size());
  EXPECT_FALSE(vector.contains(0));
  EXPECT_FALSE(vector.contains(1));
  EXPECT_EQ(-1, vector.index_of(0));
  EXPECT_EQ(-1, vector.index_of(1));

  // Push 1, vector = [1].
  vector.push_back(1);
  EXPECT_FALSE(vector.empty());
  EXPECT_EQ(1, vector.size());
  EXPECT_EQ(1, vector.front());
  EXPECT_EQ(1, vector.back());
  EXPECT_EQ(1, vector.data()[0]);
  EXPECT_EQ(1, vector[0]);
  EXPECT_EQ(1, vector.at(0));
  EXPECT_FALSE(vector.contains(0));
  EXPECT_TRUE(vector.contains(1));
  EXPECT_EQ(-1, vector.index_of(0));
  EXPECT_EQ(0, vector.index_of(1));

  // Push 0, vector = [1, 0].
  vector.push_back(0);
  EXPECT_FALSE(vector.empty());
  EXPECT_EQ(2, vector.size());
  EXPECT_EQ(1, vector.front());
  EXPECT_EQ(0, vector.back());
  EXPECT_EQ(1, vector.data()[0]);
  EXPECT_EQ(0, vector.data()[1]);
  EXPECT_EQ(1, vector[0]);
  EXPECT_EQ(0, vector[1]);
  EXPECT_EQ(1, vector.at(0));
  EXPECT_EQ(0, vector.at(1));
  EXPECT_TRUE(vector.contains(0));
  EXPECT_TRUE(vector.contains(1));
  EXPECT_EQ(1, vector.index_of(0));
  EXPECT_EQ(0, vector.index_of(1));

  // Erase 1, vector = [0].
  vector.erase(1);
  EXPECT_FALSE(vector.empty());
  EXPECT_EQ(1, vector.size());
  EXPECT_EQ(0, vector.front());
  EXPECT_EQ(0, vector.back());
  EXPECT_EQ(0, vector.data()[0]);
  EXPECT_EQ(0, vector[0]);
  EXPECT_EQ(0, vector.at(0));
  EXPECT_TRUE(vector.contains(0));
  EXPECT_FALSE(vector.contains(1));
  EXPECT_EQ(0, vector.index_of(0));
  EXPECT_EQ(-1, vector.index_of(1));

  // Clear, vector = [].
  vector.clear();
  EXPECT_TRUE(vector.empty());
  EXPECT_EQ(0, vector.size());
  EXPECT_EQ(2, vector.max_size());
  EXPECT_FALSE(vector.contains(0));
  EXPECT_FALSE(vector.contains(1));
  EXPECT_EQ(-1, vector.index_of(0));
  EXPECT_EQ(-1, vector.index_of(1));

  // Push 2 and 3, vector = [2, 3].
  vector.push_back(2);
  vector.push_back(3);
  EXPECT_FALSE(vector.empty());
  EXPECT_EQ(2, vector.size());
  EXPECT_EQ(2, vector.front());
  EXPECT_EQ(3, vector.back());
  EXPECT_EQ(2, vector[0]);
  EXPECT_EQ(3, vector[1]);

  // Pop back, vector = [2].
  vector.pop_back();
  EXPECT_FALSE(vector.empty());
  EXPECT_EQ(1, vector.size());
  EXPECT_EQ(2, vector.front());
  EXPECT_EQ(2, vector.back());
  EXPECT_EQ(2, vector[0]);
}

// Tests the various ways of initializing a vector.
TEST(FixedVectorTest, Initializers) {
  FixedVector<int, 3> vector;
  EXPECT_TRUE(vector.empty());
  EXPECT_EQ(0, vector.size());
  EXPECT_EQ(3, vector.max_size());

  int elements[2] = {1, 2};
  vector = {elements, 2};
  EXPECT_FALSE(vector.empty());
  EXPECT_EQ(2, vector.size());
  EXPECT_EQ(3, vector.max_size());
  EXPECT_EQ(1, vector[0]);
  EXPECT_EQ(2, vector[1]);

  int elements_cc[2] = {2, 3};
  vector = {elements_cc};
  EXPECT_FALSE(vector.empty());
  EXPECT_EQ(2, vector.size());
  EXPECT_EQ(3, vector.max_size());
  EXPECT_EQ(2, vector[0]);
  EXPECT_EQ(3, vector[1]);

  std::array<int, 2> elements_array = {{3, 4}};
  vector = {elements_array};
  EXPECT_FALSE(vector.empty());
  EXPECT_EQ(2, vector.size());
  EXPECT_EQ(3, vector.max_size());
  EXPECT_EQ(3, vector[0]);
  EXPECT_EQ(4, vector[1]);

  vector = {{4, 5}};
  EXPECT_FALSE(vector.empty());
  EXPECT_EQ(2, vector.size());
  EXPECT_EQ(3, vector.max_size());
  EXPECT_EQ(4, vector[0]);
  EXPECT_EQ(5, vector[1]);

  std::vector<int> elements_vector = {5, 6};
  vector = {elements_vector};
  EXPECT_FALSE(vector.empty());
  EXPECT_EQ(2, vector.size());
  EXPECT_EQ(3, vector.max_size());
  EXPECT_EQ(5, vector[0]);
  EXPECT_EQ(6, vector[1]);
}

// Tests copying an entire vector.
TEST(FixedVectorTest, Copying) {
  FixedVector<int, 3> vector_a = {{1, 2}};
  FixedVector<int, 3> vector_b = {{3, 4}};
  vector_a = vector_b;
  EXPECT_EQ(3, vector_a[0]);
  EXPECT_EQ(4, vector_a[1]);
  EXPECT_EQ(3, vector_b[0]);
  EXPECT_EQ(4, vector_b[1]);
}

// Tests swapping an entire vector.
TEST(FixedVectorTest, Swapping) {
  FixedVector<int, 3> vector_a = {{1, 2}};
  FixedVector<int, 3> vector_b = {{3, 4}};
  std::swap(vector_a, vector_b);
  EXPECT_EQ(3, vector_a[0]);
  EXPECT_EQ(4, vector_a[1]);
  EXPECT_EQ(1, vector_b[0]);
  EXPECT_EQ(2, vector_b[1]);
}

// Tests iterator usage.
TEST(FixedVectorTest, Iterators) {
  FixedVector<int, 3> empty;
  EXPECT_EQ(empty.begin(), empty.end());
  EXPECT_FALSE(empty.begin() != empty.end());
  bool found_any = false;
  for (auto value : empty) {
    EXPECT_EQ(0, value);
    found_any = true;
  }
  EXPECT_FALSE(found_any);

  FixedVector<int, 3> vector = {1, 2};
  EXPECT_NE(vector.begin(), vector.end());
  EXPECT_FALSE(vector.begin() == vector.end());
  int index = 0;
  for (auto value : vector) {
    if (index == 0) {
      EXPECT_EQ(1, value);
    } else if (index == 1) {
      EXPECT_EQ(2, value);
    } else {
      FAIL();
    }
    ++index;
  }
  if (index != 2) {
    FAIL();
  }
}

struct MyType {
  MyType() { ++ctor_called; }
  ~MyType() { ++dtor_called; }

  static int ctor_called;
  static int dtor_called;
};
int MyType::ctor_called = 0;
int MyType::dtor_called = 0;

// Tests that element constructors and destructors are called at the right
// times.
TEST(FixedVectorTest, CtorsAndDtors) {
  // Empty list calls initializers.
  MyType::ctor_called = 0;
  MyType::dtor_called = 0;
  {
    FixedVector<MyType, 3> empty;
    EXPECT_EQ(3, MyType::ctor_called);
    EXPECT_EQ(0, MyType::dtor_called);
  }
  EXPECT_EQ(3, MyType::ctor_called);
  EXPECT_EQ(3, MyType::dtor_called);

  // Empty list + 2 new items replacing old ones.
  MyType::ctor_called = 0;
  MyType::dtor_called = 0;
  {
    FixedVector<MyType, 3> vector;
    EXPECT_EQ(3, MyType::ctor_called);
    EXPECT_EQ(0, MyType::dtor_called);
    vector.push_back({});
    EXPECT_EQ(4, MyType::ctor_called);
    EXPECT_EQ(1, MyType::dtor_called);
    vector.push_back({});
    EXPECT_EQ(5, MyType::ctor_called);
    EXPECT_EQ(2, MyType::dtor_called);
  }
  EXPECT_EQ(5, MyType::ctor_called);
  EXPECT_EQ(5, MyType::dtor_called);

  // Erasing resets.
  MyType::ctor_called = 0;
  MyType::dtor_called = 0;
  {
    FixedVector<MyType, 3> vector;
    EXPECT_EQ(3, MyType::ctor_called);
    EXPECT_EQ(0, MyType::dtor_called);
    vector.push_back({});
    EXPECT_EQ(4, MyType::ctor_called);
    EXPECT_EQ(1, MyType::dtor_called);
    vector.pop_back();
    EXPECT_EQ(5, MyType::ctor_called);
    EXPECT_EQ(2, MyType::dtor_called);
  }
  EXPECT_EQ(5, MyType::ctor_called);
  EXPECT_EQ(5, MyType::dtor_called);
}

}  // namespace
}  // namespace xrtl
