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

#include "xrtl/base/ref_ptr.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace {

class MyType : public RefObject<MyType> {
 public:
  int x = 5;

  using RefObject<MyType>::counter_;
};

TEST(RefPtrTest, Construction) {
  // Empty.
  ref_ptr<MyType> n1;
  EXPECT_EQ(nullptr, n1.get());
  ref_ptr<MyType> n2(nullptr);
  EXPECT_EQ(nullptr, n2.get());

  // Assign a new ptr and add ref.
  MyType* a_ptr = new MyType();
  EXPECT_EQ(0, a_ptr->counter_);
  ref_ptr<MyType> a(a_ptr);
  EXPECT_EQ(1, a->counter_);

  // Assign existing ptr without adding a ref.
  ref_ptr<MyType> b(a_ptr, false);
  EXPECT_EQ(1, b->counter_);

  // Construct from ref_ptr.
  ref_ptr<MyType> c(b);
  EXPECT_EQ(2, c->counter_);

  b.release();
}

TEST(RefPtrTest, Reset) {
  ref_ptr<MyType> a(new MyType());
  ref_ptr<MyType> b(new MyType());

  // Reset to drop reference.
  ref_ptr<MyType> a_copy = a;
  EXPECT_EQ(2, a_copy->counter_);
  a.reset();
  EXPECT_EQ(1, a_copy->counter_);

  // Reset to assign.
  a.reset(a_copy.get());
  EXPECT_EQ(2, a_copy->counter_);

  // Reset via = operator.
  a = nullptr;
  EXPECT_EQ(1, a_copy->counter_);
  a = a_copy;
  EXPECT_EQ(2, a_copy->counter_);

  // No-op on empty ptrs.
  ref_ptr<MyType> n;
  n.reset();
  n.reset(nullptr);
  n.assign(nullptr);
}

TEST(RefPtrTest, ReleaseAssign) {
  ref_ptr<MyType> a(new MyType());

  // Release a's pointer.
  MyType* a_raw_ptr = a.get();
  MyType* a_ptr = a.release();
  EXPECT_EQ(a_raw_ptr, a_ptr);
  EXPECT_EQ(nullptr, a.get());
  EXPECT_EQ(1, a_ptr->counter_);

  // Re-wrap in a ref_ptr.
  a.assign(a_ptr);
  EXPECT_EQ(1, a->counter_);

  // No-op on empty ptrs.
  ref_ptr<MyType> n;
  EXPECT_EQ(nullptr, n.release());
}

TEST(RefPtrTest, Accessors) {
  ref_ptr<MyType> a(new MyType());
  EXPECT_EQ(5, a->x);
  a->x = 100;
  EXPECT_EQ(100, a->x);

  MyType& ra = *a;
  ra.x = 200;
  EXPECT_EQ(200, ra.x);

  const MyType& cra = *a;
  EXPECT_EQ(200, cra.x);
}

TEST(RefPtrTest, BooleanExpressions) {
  ref_ptr<MyType> a(new MyType());
  ref_ptr<MyType> n;

  EXPECT_NE(nullptr, a.get());
  EXPECT_TRUE(a);
  EXPECT_FALSE(!a);
  EXPECT_EQ(true, static_cast<bool>(a));

  EXPECT_EQ(nullptr, n.get());
  EXPECT_FALSE(n);
  EXPECT_TRUE(!n);
  EXPECT_EQ(false, static_cast<bool>(n));
}

TEST(RefPtrTest, Comparisons) {
  ref_ptr<MyType> a(new MyType());
  ref_ptr<MyType> b(new MyType());
  ref_ptr<MyType> n;

  EXPECT_TRUE(a == a);
  EXPECT_TRUE(a == a.get());
  EXPECT_TRUE(a.get() == a);
  EXPECT_FALSE(a != a);
  EXPECT_FALSE(a != a.get());
  EXPECT_FALSE(a.get() != a);

  EXPECT_FALSE(a == b);
  EXPECT_FALSE(a == b.get());
  EXPECT_FALSE(a.get() == b);
  EXPECT_TRUE(a != b);
  EXPECT_TRUE(a != b.get());
  EXPECT_TRUE(a.get() != b);

  EXPECT_TRUE(n == n);
  EXPECT_TRUE(n == n.get());
  EXPECT_TRUE(n.get() == n);
  EXPECT_FALSE(n != n);
  EXPECT_FALSE(n != n.get());
  EXPECT_FALSE(n.get() != n);

  EXPECT_FALSE(a < a);
  EXPECT_TRUE(n < a);
}

TEST(RefPtrTest, Swap) {
  ref_ptr<MyType> a(new MyType());
  ref_ptr<MyType> b(new MyType());
  MyType* a_ptr = a.get();
  MyType* b_ptr = b.get();

  swap(a, a);
  EXPECT_EQ(a_ptr, a);

  swap(a, b);
  EXPECT_EQ(a_ptr, b.get());
  EXPECT_EQ(b_ptr, a.get());

  swap(a, b);
  EXPECT_EQ(a_ptr, a.get());
  EXPECT_EQ(b_ptr, b.get());

  ref_ptr<MyType> c;
  swap(a, c);
  EXPECT_EQ(a_ptr, c.get());
  EXPECT_EQ(nullptr, a.get());
}

class DefaultDeleterType : public RefObject<DefaultDeleterType> {
 public:
  DefaultDeleterType() { ++alloc_count; }
  ~DefaultDeleterType() { --alloc_count; }
  static int alloc_count;
};
int DefaultDeleterType::alloc_count = 0;

TEST(RefPtrTest, DefaultDeleter) {
  // Empty is ok.
  ref_ptr<DefaultDeleterType> n;
  n.reset();

  // Lifecycle.
  EXPECT_EQ(0, DefaultDeleterType::alloc_count);
  ref_ptr<DefaultDeleterType> a = make_ref<DefaultDeleterType>();
  EXPECT_EQ(1, DefaultDeleterType::alloc_count);
  a.reset();
  EXPECT_EQ(0, DefaultDeleterType::alloc_count);
}

class CustomDeleterType : public RefObject<CustomDeleterType> {
 public:
  CustomDeleterType() { ++alloc_count; }
  static void Delete(CustomDeleterType* ptr) {
    --alloc_count;
    ::operator delete(ptr);
  }
  static int alloc_count;
};
int CustomDeleterType::alloc_count = 0;

TEST(RefPtrTest, InlineDeallocator) {
  // Empty is ok.
  ref_ptr<CustomDeleterType> n;
  n.reset();

  // Lifecycle.
  EXPECT_EQ(0, CustomDeleterType::alloc_count);
  ref_ptr<CustomDeleterType> a = make_ref<CustomDeleterType>();
  EXPECT_EQ(1, CustomDeleterType::alloc_count);
  a.reset();
  EXPECT_EQ(0, CustomDeleterType::alloc_count);
}

class VirtualDtorTypeA : public RefObject<VirtualDtorTypeA> {
 public:
  VirtualDtorTypeA() { ++alloc_count_a; }
  virtual ~VirtualDtorTypeA() { --alloc_count_a; }
  static int alloc_count_a;
};
int VirtualDtorTypeA::alloc_count_a = 0;

class VirtualDtorTypeB : public VirtualDtorTypeA {
 public:
  VirtualDtorTypeB() { ++alloc_count_b; }
  ~VirtualDtorTypeB() override { --alloc_count_b; }
  static int alloc_count_b;
};
int VirtualDtorTypeB::alloc_count_b = 0;

TEST(RefPtrTest, VirtualDestructor) {
  // Empty is ok.
  ref_ptr<VirtualDtorTypeB> n;
  n.reset();

  // Lifecycle.
  EXPECT_EQ(0, VirtualDtorTypeA::alloc_count_a);
  EXPECT_EQ(0, VirtualDtorTypeB::alloc_count_b);
  ref_ptr<VirtualDtorTypeB> a = make_ref<VirtualDtorTypeB>();
  EXPECT_EQ(1, VirtualDtorTypeA::alloc_count_a);
  EXPECT_EQ(1, VirtualDtorTypeB::alloc_count_b);
  a.reset();
  EXPECT_EQ(0, VirtualDtorTypeA::alloc_count_a);
  EXPECT_EQ(0, VirtualDtorTypeB::alloc_count_b);
}

}  // namespace
}  // namespace xrtl
