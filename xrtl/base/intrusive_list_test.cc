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

#include "xrtl/base/intrusive_list.h"

#include <functional>
#include <vector>

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace {

using ::testing::ElementsAre;

struct Item {
  size_t some_data_0;
  IntrusiveListLink list_a;
  size_t some_data_1;
  IntrusiveListLink list_b;
  size_t some_data_2;
  int value;

  static const size_t kToken = 0xDEADBEEF;
  explicit Item(int value)
      : some_data_0(kToken),
        some_data_1(kToken),
        some_data_2(kToken),
        value(value) {}
  bool is_valid() {
    return some_data_0 == kToken && some_data_1 == kToken &&
           some_data_2 == kToken;
  }
};

template <typename T, size_t V>
std::vector<T*> ExtractItems(const IntrusiveList<T, V>& list) {
  std::vector<T*> items;
  for (auto* item : list) {
    items.push_back(item);
  }
  return items;
}

template <typename T, size_t V>
std::vector<int> ExtractValues(const IntrusiveList<T, V>& list) {
  std::vector<int> values;
  for (auto* item : list) {
    values.push_back(item->value);
  }
  return values;
}

template <typename T, size_t V>
std::vector<int> ExtractValuesMutable(const IntrusiveList<T, V>& list) {
  std::vector<int> values;
  for (auto* item : list) {
    values.push_back(item->value);
  }
  return values;
}

TEST(IntrusiveListTest, PushPopItems) {
  Item item1(1);
  Item item2(2);
  Item item3(3);
  Item item4(4);

  IntrusiveList<Item, offsetof(Item, list_a)> items;
  EXPECT_TRUE(items.empty());
  EXPECT_EQ(items.size(), 0u);
  EXPECT_EQ(items.front(), nullptr);
  EXPECT_EQ(items.back(), nullptr);
  EXPECT_TRUE(items.begin() == items.end());
  items.push_front(&item1);
  EXPECT_FALSE(items.empty());
  EXPECT_EQ(items.size(), 1u);
  EXPECT_EQ(items.front(), &item1);
  EXPECT_EQ(items.back(), &item1);
  EXPECT_FALSE(items.begin() == items.end());
  items.push_front(&item2);
  EXPECT_EQ(items.size(), 2u);
  EXPECT_EQ(items.front(), &item2);
  EXPECT_EQ(items.back(), &item1);
  items.push_front(&item3);
  EXPECT_EQ(items.size(), 3u);
  EXPECT_EQ(items.front(), &item3);
  EXPECT_EQ(items.back(), &item1);
  EXPECT_THAT(ExtractValues(items), ElementsAre(3, 2, 1));

  items.push_back(&item4);
  EXPECT_EQ(items.size(), 4u);
  EXPECT_EQ(items.front(), &item3);
  EXPECT_EQ(items.back(), &item4);
  EXPECT_THAT(ExtractValues(items), ElementsAre(3, 2, 1, 4));

  items.pop_front();
  EXPECT_EQ(items.size(), 3u);
  EXPECT_EQ(items.front(), &item2);
  EXPECT_EQ(items.back(), &item4);
  EXPECT_THAT(ExtractValues(items), ElementsAre(2, 1, 4));

  items.pop_back();
  EXPECT_EQ(items.size(), 2u);
  EXPECT_EQ(items.front(), &item2);
  EXPECT_EQ(items.back(), &item1);
  EXPECT_THAT(ExtractValues(items), ElementsAre(2, 1));

  items.pop_back();
  items.pop_back();
  EXPECT_TRUE(items.empty());
  EXPECT_EQ(items.size(), 0u);
  EXPECT_EQ(items.front(), nullptr);
  EXPECT_EQ(items.back(), nullptr);
  EXPECT_TRUE(items.begin() == items.end());

  EXPECT_TRUE(item1.is_valid());
  EXPECT_TRUE(item2.is_valid());
  EXPECT_TRUE(item3.is_valid());
  EXPECT_TRUE(item4.is_valid());
}

TEST(IntrusiveListTest, Erase) {
  Item item1(1);
  Item item2(2);
  Item item3(3);
  Item item4(4);

  IntrusiveList<Item, offsetof(Item, list_a)> items;
  items.push_back(&item1);
  items.push_back(&item2);
  items.push_back(&item3);
  items.push_back(&item4);

  EXPECT_THAT(ExtractValues(items), ElementsAre(1, 2, 3, 4));
  items.erase(&item3);
  EXPECT_THAT(ExtractValues(items), ElementsAre(1, 2, 4));
  items.erase(&item1);
  EXPECT_THAT(ExtractValues(items), ElementsAre(2, 4));
  items.erase(&item4);
  EXPECT_THAT(ExtractValues(items), ElementsAre(2));
  items.erase(&item2);
  EXPECT_TRUE(items.empty());

  items.push_back(&item1);
  items.push_back(&item2);
  items.push_back(&item3);
  items.push_back(&item4);

  EXPECT_THAT(ExtractValues(items), ElementsAre(1, 2, 3, 4));
  auto it = items.begin();
  items.erase(it);
  EXPECT_THAT(ExtractValues(items), ElementsAre(2, 3, 4));
  it = items.end();
  items.erase(it);
  EXPECT_THAT(ExtractValues(items), ElementsAre(2, 3, 4));
  it = items.begin();
  ++it;
  items.erase(it);
  EXPECT_THAT(ExtractValues(items), ElementsAre(2, 4));

  it = items.begin();
  it = items.erase(it);
  EXPECT_EQ(4, (*it)->value);
  EXPECT_THAT(ExtractValues(items), ElementsAre(4));
  it = items.erase(it);
  EXPECT_TRUE(items.empty());
  EXPECT_EQ(items.end(), it);
}

TEST(IntrusiveListTest, MultipleLists) {
  Item item1(1);
  Item item2(2);
  Item item3(3);
  Item item4(4);

  IntrusiveList<Item, offsetof(Item, list_a)> items_a;
  IntrusiveList<Item, offsetof(Item, list_b)> items_b;
  items_a.push_back(&item1);
  items_a.push_back(&item2);
  items_a.push_back(&item3);
  items_a.push_back(&item4);
  items_b.push_front(&item1);
  items_b.push_front(&item2);
  items_b.push_front(&item3);
  items_b.push_front(&item4);
  EXPECT_THAT(ExtractValues(items_a), ElementsAre(1, 2, 3, 4));
  EXPECT_THAT(ExtractValues(items_b), ElementsAre(4, 3, 2, 1));
  items_b.erase(&item3);
  EXPECT_THAT(ExtractValues(items_a), ElementsAre(1, 2, 3, 4));
  EXPECT_THAT(ExtractValues(items_b), ElementsAre(4, 2, 1));
  items_a.pop_back();
  EXPECT_THAT(ExtractValues(items_a), ElementsAre(1, 2, 3));
  EXPECT_THAT(ExtractValues(items_b), ElementsAre(4, 2, 1));
}

TEST(IntrusiveListTest, MutableIterator) {
  Item item1(1);
  Item item2(2);
  Item item3(3);
  Item item4(4);

  IntrusiveList<Item, offsetof(Item, list_a)> items;
  items.push_back(&item4);
  items.push_front(&item1);
  items.push_front(&item2);
  items.push_front(&item3);

  EXPECT_THAT(ExtractValuesMutable(items), ElementsAre(3, 2, 1, 4));
}

struct BaseType {
  explicit BaseType(int value) : value(value) {}
  int value;
  IntrusiveListLink base_link;
};
struct SubType : public BaseType {
  explicit SubType(int value) : BaseType(value) {}
  IntrusiveListLink sub_link;
};
TEST(IntrusiveListTest, SimpleType) {
  SubType item1(1);
  SubType item2(2);
  SubType item3(3);
  SubType item4(4);

  IntrusiveList<BaseType, offsetof(BaseType, base_link)> items_a;
  items_a.push_front(&item1);
  items_a.push_front(&item2);
  items_a.push_front(&item3);
  items_a.push_front(&item4);
  EXPECT_THAT(ExtractValues(items_a), ElementsAre(4, 3, 2, 1));

  IntrusiveList<SubType, offsetof(SubType, sub_link)> items_b;
  items_b.push_back(&item1);
  items_b.push_back(&item2);
  items_b.push_back(&item3);
  items_b.push_back(&item4);
  EXPECT_THAT(ExtractValues(items_b), ElementsAre(1, 2, 3, 4));
}

struct AbstractType {
  explicit AbstractType(int value) : value(value) {}
  virtual ~AbstractType() = default;
  virtual int DoSomething() = 0;
  int value;
  IntrusiveListLink base_link;
};
struct ImplType : public AbstractType {
  explicit ImplType(int value) : AbstractType(value) {}
  int DoSomething() override { return value; }
  IntrusiveListLink sub_link;
};

TEST(IntrusiveListTest, ComplexType) {
  ImplType item1(1);
  ImplType item2(2);
  ImplType item3(3);
  ImplType item4(4);

  IntrusiveList<AbstractType, offsetof(AbstractType, base_link)> items_a;
  items_a.push_front(&item1);
  items_a.push_front(&item2);
  items_a.push_front(&item3);
  items_a.push_front(&item4);
  EXPECT_THAT(ExtractValues(items_a), ElementsAre(4, 3, 2, 1));

  IntrusiveList<ImplType, offsetof(ImplType, sub_link)> items_b;
  items_b.push_back(&item1);
  items_b.push_back(&item2);
  items_b.push_back(&item3);
  items_b.push_back(&item4);
  EXPECT_THAT(ExtractValues(items_b), ElementsAre(1, 2, 3, 4));
}

bool Comparison(Item* a, Item* b) { return a->value < b->value; }

TEST(IntrusiveListTest, Inserting) {
  Item item1(1);
  Item item2(2);
  Item item3(3);
  Item item4(4);

  IntrusiveList<Item, offsetof(Item, list_a)> items;
  items.insert(items.end(), &item3);
  items.insert(items.begin(), &item1);
  items.insert(items.end(), &item4);

  auto pos = std::upper_bound(items.begin(), items.end(), &item2, Comparison);
  items.insert(pos, &item2);

  EXPECT_THAT(ExtractValues(items), ElementsAre(1, 2, 3, 4));
}

// TODO(benvanik): test reverse iteration.
// TODO(benvanik): test clear.
// TODO(benvanik): test replace.

TEST(IntrusiveListTest, Sort) {
  Item item1(1);
  Item item2(2);
  Item item3(3);
  Item item4(4);

  IntrusiveList<Item, offsetof(Item, list_a)> items;

  // Empty sort.
  items.sort([](Item* a, Item* b) { return a->value < b->value; });

  // Single item sort.
  items.clear();
  items.push_back(&item1);
  items.sort([](Item* a, Item* b) { return a->value < b->value; });
  EXPECT_THAT(ExtractValues(items), ElementsAre(1));

  // Already sorted.
  items.clear();
  items.push_back(&item1);
  items.push_back(&item2);
  items.push_back(&item3);
  items.push_back(&item4);
  items.sort([](Item* a, Item* b) { return a->value < b->value; });
  EXPECT_THAT(ExtractValues(items), ElementsAre(1, 2, 3, 4));

  // Reverse.
  items.clear();
  items.push_back(&item4);
  items.push_back(&item3);
  items.push_back(&item2);
  items.push_back(&item1);
  items.sort([](Item* a, Item* b) { return a->value < b->value; });
  EXPECT_THAT(ExtractValues(items), ElementsAre(1, 2, 3, 4));

  // Random.
  items.clear();
  items.push_back(&item2);
  items.push_back(&item4);
  items.push_back(&item1);
  items.push_back(&item3);
  items.sort([](Item* a, Item* b) { return a->value < b->value; });
  EXPECT_THAT(ExtractValues(items), ElementsAre(1, 2, 3, 4));

  // Stability.
  Item item1a(1);
  Item item2a(2);
  items.clear();
  items.push_back(&item2);
  items.push_back(&item4);
  items.push_back(&item1);
  items.push_back(&item3);
  items.push_back(&item1a);
  items.push_back(&item2a);
  items.sort([](Item* a, Item* b) { return a->value <= b->value; });
  EXPECT_THAT(ExtractValues(items), ElementsAre(1, 1, 2, 2, 3, 4));
  auto items_vector = ExtractItems(items);
  EXPECT_EQ(&item1, items_vector[0]);
  EXPECT_EQ(&item1a, items_vector[1]);
  EXPECT_EQ(&item2, items_vector[2]);
  EXPECT_EQ(&item2a, items_vector[3]);

  items.clear();
}

struct AllocatedType : public IntrusiveLinkBase<void> {
  AllocatedType() { ++alloc_count; }
  ~AllocatedType() { --alloc_count; }
  static int alloc_count;
};
int AllocatedType::alloc_count = 0;

TEST(IntrusiveListTest, UniquePtr) {
  AllocatedType::alloc_count = 0;

  // Push/clear.
  IntrusiveList<std::unique_ptr<AllocatedType>> list;
  EXPECT_EQ(0, AllocatedType::alloc_count);
  list.push_back(absl::make_unique<AllocatedType>());
  EXPECT_EQ(1, AllocatedType::alloc_count);
  EXPECT_NE(nullptr, list.front());
  list.clear();
  EXPECT_EQ(0, AllocatedType::alloc_count);

  // Push/pop.
  list.push_back(absl::make_unique<AllocatedType>());
  EXPECT_EQ(1, AllocatedType::alloc_count);
  EXPECT_NE(nullptr, list.front());
  for (auto item : list) {
    EXPECT_EQ(item, list.front());
  }
  list.pop_back();
  EXPECT_EQ(0, AllocatedType::alloc_count);

  // Push/take.
  list.push_back(absl::make_unique<AllocatedType>());
  EXPECT_EQ(1, AllocatedType::alloc_count);
  EXPECT_NE(nullptr, list.front());
  auto item = list.take(list.front());
  EXPECT_TRUE(list.empty());
  EXPECT_NE(nullptr, item.get());
  EXPECT_EQ(1, AllocatedType::alloc_count);
  item.reset();
  EXPECT_EQ(0, AllocatedType::alloc_count);

  // Push/replace.
  list.push_back(absl::make_unique<AllocatedType>());
  EXPECT_EQ(1, AllocatedType::alloc_count);
  list.replace(list.front(), absl::make_unique<AllocatedType>());
  EXPECT_EQ(1, AllocatedType::alloc_count);
  list.clear();
  EXPECT_EQ(0, AllocatedType::alloc_count);

  // Iteration.
  list.push_back(absl::make_unique<AllocatedType>());
  list.push_back(absl::make_unique<AllocatedType>());
  list.push_back(absl::make_unique<AllocatedType>());
  EXPECT_EQ(3, AllocatedType::alloc_count);
  for (auto item : list) {
    AllocatedType* item_ptr = item;
    EXPECT_NE(nullptr, item_ptr);
  }
  list.clear();
  EXPECT_EQ(0, AllocatedType::alloc_count);
}

struct RefCountedType : public RefObject<RefCountedType> {
  IntrusiveListLink link;
  RefCountedType() { ++alloc_count; }
  ~RefCountedType() { --alloc_count; }
  static int alloc_count;
  static void Deallocate(RefCountedType* value) { delete value; }
  using RefObject<RefCountedType>::counter_;
};
int RefCountedType::alloc_count = 0;

TEST(IntrusiveListTest, RefPtr) {
  RefCountedType::alloc_count = 0;

  // Push/clear.
  IntrusiveList<ref_ptr<RefCountedType>> list;
  EXPECT_EQ(0, RefCountedType::alloc_count);
  list.push_back(make_ref<RefCountedType>());
  EXPECT_EQ(1, RefCountedType::alloc_count);
  EXPECT_NE(nullptr, list.front());
  EXPECT_EQ(2, list.front()->counter_);
  list.clear();
  EXPECT_EQ(0, RefCountedType::alloc_count);

  // Push/pop.
  list.push_back(make_ref<RefCountedType>());
  EXPECT_EQ(1, RefCountedType::alloc_count);
  EXPECT_NE(nullptr, list.front());
  for (auto item : list) {
    EXPECT_EQ(item, list.front());
  }
  list.pop_back();
  EXPECT_EQ(0, RefCountedType::alloc_count);

  // Push/erase.
  list.push_back(make_ref<RefCountedType>());
  EXPECT_EQ(1, RefCountedType::alloc_count);
  EXPECT_NE(nullptr, list.front());
  EXPECT_EQ(2, list.front()->counter_);
  auto item = list.front();
  EXPECT_NE(nullptr, item.get());
  EXPECT_EQ(3, list.front()->counter_);
  EXPECT_EQ(1, RefCountedType::alloc_count);
  list.erase(item);
  EXPECT_EQ(1, RefCountedType::alloc_count);
  item.reset();
  EXPECT_EQ(0, RefCountedType::alloc_count);

  // Push/replace.
  list.push_back(make_ref<RefCountedType>());
  EXPECT_EQ(1, RefCountedType::alloc_count);
  list.replace(list.front(), make_ref<RefCountedType>());
  EXPECT_EQ(1, RefCountedType::alloc_count);
  list.clear();
  EXPECT_EQ(0, RefCountedType::alloc_count);

  // Iteration.
  list.push_back(make_ref<RefCountedType>());
  list.push_back(make_ref<RefCountedType>());
  list.push_back(make_ref<RefCountedType>());
  EXPECT_EQ(3, RefCountedType::alloc_count);
  for (auto item : list) {
    ref_ptr<RefCountedType> item_ref = item;
    EXPECT_NE(nullptr, item_ref.get());
  }
  list.clear();
  EXPECT_EQ(0, RefCountedType::alloc_count);
}

}  // namespace
}  // namespace xrtl
