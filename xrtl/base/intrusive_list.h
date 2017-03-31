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

#ifndef XRTL_BASE_INTRUSIVE_LIST_H_
#define XRTL_BASE_INTRUSIVE_LIST_H_

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>

#include "xrtl/base/logging.h"
#include "xrtl/base/macros.h"
#include "xrtl/base/ref_ptr.h"

namespace xrtl {

// Doubly linked list using element interior storage.
// This has the performance of std::list (that means O(1) on insert and remove)
// but performs no allocations and has better caching behavior.
//
// Elements are maintained in lists by way of IntrusiveListLinks, with each link
// allowing the element to exist in one list simultaneously. In the most simple
// case subclassing IntrusiveLinkBase will let the type be added to a list with
// little boilerplate. If an element must be in more than one list
// simultaneously IntrusiveListLinks can be added as members.
//
// Usage (simple):
//   class MySimpleElement : public IntrusiveLinkBase {};
//   IntrusiveList<MySimpleElement> list;
//   list.push_back(new MySimpleElement());
//   for (auto element : list) { ... }
//
// Usage (multiple lists):
//   class MultiElement {
//    public:
//     IntrusiveListLink list_link_a;
//     IntrusiveListLink list_link_b;
//   };
//   IntrusiveList<MultiElement, offsetof(MultiElement, list_link_a)> list_a;
//   IntrusiveList<MultiElement, offsetof(MultiElement, list_link_b)> list_b;
//
// By default elements in the list are not retained and must be kept alive
// externally. For automatic memory management there are specializations for
// both std::unique_ptr and ref_ptr.
//
// Usage (unique_ptr):
//   IntrusiveList<std::unique_ptr<MyElement>> list;
//   list.push_back(make_unique<MyElement>());
//   std::unique_ptr<MyElement> elm = list.take(list.front());
//
// Usage (ref_ptr):
//   IntrusiveList<ref_ptr<MyElement>> list;
//   list.push_back(make_ref<MyElement>());
//   ref_ptr<MyElement> elm = list.front();

// Define to enable extensive checks after each mutation of the intrusive list.
// #define XRTL_PARANOID_INTRUSIVE_LIST

// Storage for the doubly-linked list.
// This is embedded within all elements in an intrusive list.
struct IntrusiveListLink {
  IntrusiveListLink* prev;
  IntrusiveListLink* next;
  IntrusiveListLink() : prev(nullptr), next(nullptr) {}

 private:
  XRTL_DISALLOW_COPY_AND_ASSIGN(IntrusiveListLink);
};

namespace impl {
// Maps an IntrusiveListLink to its containing type T.
template <typename T>
static T* LinkToT(size_t offset, IntrusiveListLink* link) {
  if (link) {
    return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(link) - offset);
  } else {
    return nullptr;
  }
}
// Maps a containing type T to its IntrusiveListLink.
template <typename T>
static IntrusiveListLink* TToLink(size_t offset, T* value) {
  if (value) {
    return reinterpret_cast<IntrusiveListLink*>(
        reinterpret_cast<uintptr_t>(value) + offset);
  } else {
    return nullptr;
  }
}
}  // namespace impl

// Basic iterator for an IntrusiveList.
template <typename T>
class IntrusiveListIterator
    : public std::iterator<std::input_iterator_tag, int> {
 public:
  IntrusiveListIterator(size_t offset, IntrusiveListLink* current, bool forward)
      : offset_(offset), current_(current), forward_(forward) {}
  IntrusiveListIterator<T>& operator++() {
    if (current_) {
      current_ = forward_ ? current_->next : current_->prev;
    }
    return *this;
  }
  IntrusiveListIterator<T> operator++(int) {
    IntrusiveListIterator<T> tmp(current_);
    operator++();
    return tmp;
  }
  IntrusiveListIterator<T>& operator--() {
    if (current_) {
      current_ = forward_ ? current_->prev : current_->next;
    }
    return *this;
  }
  IntrusiveListIterator<T> operator--(int) {
    IntrusiveListIterator<T> tmp(current_);
    operator--();
    return tmp;
  }
  bool operator==(const IntrusiveListIterator<T>& rhs) const {
    return rhs.current_ == current_;
  }
  bool operator!=(const IntrusiveListIterator<T>& rhs) const {
    return !operator==(rhs);
  }
  T* operator*() const { return impl::LinkToT<T>(offset_, current_); }

 protected:
  size_t offset_;
  IntrusiveListLink* current_;
  bool forward_;
};

// Iterator for an IntrusiveList specialized to ref_ptr.
template <typename T>
class IntrusiveListRefPtrIterator
    : public std::iterator<std::input_iterator_tag, int> {
 public:
  IntrusiveListRefPtrIterator(size_t offset, IntrusiveListLink* current,
                              bool forward)
      : offset_(offset), current_(current), forward_(forward) {}
  IntrusiveListRefPtrIterator<T>& operator++() {
    if (current_) {
      current_ = forward_ ? current_->next : current_->prev;
    }
    return *this;
  }
  IntrusiveListRefPtrIterator<T> operator++(int) {
    IntrusiveListRefPtrIterator<T> tmp(current_);
    operator++();
    return tmp;
  }
  IntrusiveListRefPtrIterator<T>& operator--() {
    if (current_) {
      current_ = forward_ ? current_->prev : current_->next;
    }
    return *this;
  }
  IntrusiveListRefPtrIterator<T> operator--(int) {
    IntrusiveListRefPtrIterator<T> tmp(current_);
    operator--();
    return tmp;
  }
  bool operator==(const IntrusiveListRefPtrIterator<T>& rhs) const {
    return rhs.current_ == current_;
  }
  bool operator!=(const IntrusiveListRefPtrIterator<T>& rhs) const {
    return !operator==(rhs);
  }
  ref_ptr<T> operator*() const {
    return ref_ptr<T>(impl::LinkToT<T>(offset_, current_));
  }

 protected:
  size_t offset_;
  IntrusiveListLink* current_;
  bool forward_;
};

// Base type for intrusive lists.
// This is either used directly when the list is on naked pointers or
// specialized to either std::unique_ptr or ref_ptr.
template <typename T, typename IT, size_t kOffset>
class IntrusiveListBase {
 public:
  using iterator_type = IT;

  IntrusiveListBase() = default;
  ~IntrusiveListBase() { clear(); }

  // Returns true if the list is empty.
  // Performance: O(1)
  bool empty() const { return head_ == nullptr; }

  // Returns the total number of items in the list.
  // Performance: O(1)
  size_t size() const { return count_; }

  // Returns true if the given item is contained within the list.
  // Performance: O(n)
  bool contains(T* value) const {
    // TODO(benvanik): faster way of checking? requires list ptr in link?
    auto needle = impl::TToLink(offset_, value);
    auto link = head_;
    while (link) {
      if (link == needle) {
        return true;
      }
      link = link->next;
    }
    return false;
  }

  // Removes all items from the list.
  // Performance: O(n)
  void clear() {
    auto link = head_;
    while (link) {
      auto next = link->next;
      link->prev = link->next = nullptr;
      OnDeallocate(impl::LinkToT<T>(offset_, link));
      link = next;
    }
    head_ = tail_ = nullptr;
    count_ = 0;
  }

  iterator_type begin() const { return iterator_type(offset_, head_, true); }
  iterator_type end() const { return iterator_type(offset_, nullptr, true); }
  iterator_type rbegin() const { return iterator_type(offset_, tail_, false); }
  iterator_type rend() const { return iterator_type(offset_, nullptr, false); }

  // Returns the next item in the list relative to the given item.
  // Performance: O(1)
  inline T* next(T* value) const {
    if (!value) {
      return nullptr;
    }
    auto link = impl::TToLink(offset_, value);
    return impl::LinkToT<T>(offset_, link->next);
  }

  // Returns the previous item in the list relative to the given item.
  // Performance: O(1)
  inline T* previous(T* value) const {
    if (!value) {
      return nullptr;
    }
    auto link = impl::TToLink(offset_, value);
    return impl::LinkToT<T>(offset_, link->prev);
  }

  // Returns the item at the front of the list, if any.
  // Performance: O(1)
  inline T* front() const { return impl::LinkToT<T>(offset_, head_); }

  // Inserts an item at the front of the list.
  // Performance: O(1)
  void push_front(T* value) {
    DCHECK(value);
    auto link = impl::TToLink(offset_, value);
    DCHECK(!link->next);
    DCHECK(!link->prev);
    link->next = head_;
    link->prev = nullptr;
    head_ = link;
    if (link->next) {
      link->next->prev = link;
    }
    if (!tail_) {
      tail_ = link;
    }
    ++count_;
    OnAdd(value);
    CheckCorrectness();
  }

  // Removes the item at the front of the list.
  // Performance: O(1)
  void pop_front() {
    DCHECK(head_);
    auto link = head_;
    if (link) {
      head_ = head_->next;
      link->next = link->prev = nullptr;
      if (head_) {
        head_->prev = nullptr;
      }
      if (link == tail_) {
        tail_ = nullptr;
      }
      --count_;
      OnDeallocate(impl::LinkToT<T>(offset_, link));
    }
    CheckCorrectness();
  }

  // Returns the item at the back of the list, if any.
  // Performance: O(1)
  inline T* back() const { return impl::LinkToT<T>(offset_, tail_); }

  // Inserts an item at the back of the list.
  // Performance: O(1)
  void push_back(T* value) {
    DCHECK(value);
    auto link = impl::TToLink(offset_, value);
    DCHECK(!link->next);
    DCHECK(!link->prev);
    link->prev = tail_;
    link->next = nullptr;
    tail_ = link;
    if (link->prev) {
      link->prev->next = link;
    }
    if (!head_) {
      head_ = link;
    }
    ++count_;
    OnAdd(value);
    CheckCorrectness();
  }

  // Removes the item at the back of the list.
  // Performance: O(1)
  void pop_back() {
    DCHECK(tail_);
    auto link = tail_;
    if (link) {
      tail_ = tail_->prev;
      link->next = link->prev = nullptr;
      if (tail_) {
        tail_->next = nullptr;
      }
      if (link == head_) {
        head_ = nullptr;
      }
      --count_;
      OnDeallocate(impl::LinkToT<T>(offset_, link));
    }
    CheckCorrectness();
  }

  // Inserts an item into the list before the given iterator.
  // Performance: O(1)
  // NOLINT(runtime/references)
  void insert(const iterator_type& it, T* value) {
    DCHECK(value);
    auto link = impl::TToLink(offset_, value);
    auto position = impl::TToLink(offset_, *it);
    DCHECK(!link->next);
    DCHECK(!link->prev);

    if (position == head_) {
      push_front(value);
    } else if (position == nullptr) {
      push_back(value);
    } else {
      link->next = position;
      link->prev = position->prev;
      position->prev->next = link;
      position->prev = link;
      ++count_;
      OnAdd(value);
    }
    CheckCorrectness();
  }

  // Erases the item from the list at the given iterator.
  // Performance: O(1)
  iterator_type erase(iterator_type& it) {  // NOLINT(runtime/references)
    return erase(*it);
  }

  // Erases the given item from the list.
  // Performance: O(1)
  iterator_type erase(T* value) {
    if (!value) {
      return end();
    }
    auto link = impl::TToLink(offset_, value);
    if (link->prev) {
      DCHECK_NE(link, head_);
      link->prev->next = link->next;
    } else {
      DCHECK_EQ(link, head_);
      head_ = link->next;
    }
    if (link->next) {
      DCHECK_NE(link, tail_);
      link->next->prev = link->prev;
    } else {
      DCHECK_EQ(link, tail_);
      tail_ = link->prev;
    }
    auto next = link->next;
    link->next = link->prev = nullptr;
    --count_;
    OnDeallocate(value);
    CheckCorrectness();
    return iterator_type(offset_, next, true);
  }

  // Replaces the item with a new item at the same position.
  // Performance: O(1)
  void replace(T* old_value, T* new_value) {
    DCHECK(old_value);
    DCHECK(new_value);
    if (old_value == new_value) {
      return;
    }
    auto old_link = impl::TToLink(offset_, old_value);
    auto new_link = impl::TToLink(offset_, new_value);
    new_link->next = old_link->next;
    new_link->prev = old_link->prev;
    if (new_link->prev) {
      new_link->prev->next = new_link;
    } else {
      head_ = new_link;
    }
    if (new_link->next) {
      new_link->next->prev = new_link;
    } else {
      tail_ = new_link;
    }
    old_link->next = old_link->prev = nullptr;
    OnAdd(new_value);
    OnDeallocate(old_value);
    CheckCorrectness();
  }

  // Sorts the list with the given comparison function.
  // The sort function is the same as used by std::sort.
  //
  // Uses merge sort O(N log N) using the algorithm described here:
  // http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html
  void sort(bool (*compare_fn)(T* a, T* b)) {
    if (empty()) {
      // Empty list no-op.
      return;
    }
    // Repeatedly run until the list is sorted.
    int in_size = 1;
    while (true) {
      IntrusiveListLink* p = head_;
      IntrusiveListLink* q = nullptr;
      IntrusiveListLink* e = nullptr;
      IntrusiveListLink* tail = nullptr;
      head_ = nullptr;
      tail_ = nullptr;
      // Repeatedly merge sublists.
      int merge_count = 0;
      while (p) {
        ++merge_count;
        q = p;
        // Determine the size of the first part and find the second.
        int p_size = 0;
        for (int i = 0; i < in_size; ++i) {
          ++p_size;
          q = q->next;
          if (!q) {
            break;
          }
        }
        // Merge the two lists (if we have two).
        int q_size = in_size;
        while (p_size > 0 || (q_size > 0 && q)) {
          if (p_size == 0) {
            // p is empty; e must come from q.
            e = q;
            q = q->next;
            --q_size;
          } else if (q_size == 0 || !q) {
            // q is empty; e must come from p.
            e = p;
            p = p->next;
            --p_size;
          } else if (compare_fn(impl::LinkToT<T>(offset_, p),
                                impl::LinkToT<T>(offset_, q))) {
            // p <= q; e must come from p.
            e = p;
            p = p->next;
            --p_size;
          } else {
            // q < p; e must come from q.
            e = q;
            q = q->next;
            --q_size;
          }
          // Append e to the merged list.
          if (tail) {
            tail->next = e;
          } else {
            head_ = e;
          }
          e->prev = tail;
          tail = e;
        }
        p = q;
      }
      tail->next = nullptr;
      if (merge_count <= 1) {
        // List is now sorted; stash and return.
        tail_ = tail;
        CheckCorrectness();
        return;
      }
      // Run merge again with larger lists.
      in_size *= 2;
    }
  }

 protected:
  // Called when an item is added to the list.
  virtual void OnAdd(T* value) {}
  // Called when an item is removed from the list.
  virtual void OnRemove(T* value) {}
  // Called when an item is removed and deallocated.
  virtual void OnDeallocate(T* value) {}

#if defined(XRTL_PARANOID_INTRUSIVE_LIST)
  // Performs expensive correctness checks on the list structure. It's too slow
  // to use in normal builds (even dbg), so it should only be used when there's
  // a suspected issue with an intrusive list.
  void CheckCorrectness() const {
    auto link = head_;
    IntrusiveListLink* previous = nullptr;
    size_t actual_count = 0;
    while (link) {
      ++actual_count;
      if (!link->prev) {
        DCHECK_EQ(link, head_);
      }
      if (!link->next) {
        DCHECK_EQ(link, tail_);
      }
      DCHECK_EQ(link->prev, previous);
      previous = link;
      link = link->next;
    }
    DCHECK_EQ(actual_count, count_);
  }
#else
  void CheckCorrectness() const {}
#endif  // XRTL_PARANOID_INTRUSIVE_LIST

  size_t offset_ = kOffset;
  IntrusiveListLink* head_ = nullptr;
  IntrusiveListLink* tail_ = nullptr;
  size_t count_ = 0;

  XRTL_DISALLOW_COPY_AND_ASSIGN(IntrusiveListBase);
};

// Specialized IntrusiveListBase used for unreferenced naked pointers.
// This very thinly wraps the base type and does no special memory management.
template <typename T, size_t kOffset>
class IntrusiveListUnrefBase
    : public IntrusiveListBase<T, IntrusiveListIterator<T>, kOffset> {
 public:
  using iterator_type = IntrusiveListIterator<T>;
  using base_list = IntrusiveListBase<T, iterator_type, kOffset>;

  using base_list::clear;

  // Removes all items from the list and calls the given deleter function for
  // each of them.
  // Performance: O(n)
  void clear(std::function<void(T*)> deleter) {
    auto link = head_;
    while (link) {
      auto next = link->next;
      link->prev = link->next = nullptr;
      deleter(impl::LinkToT<T>(offset_, link));
      base_list::OnDeallocate(link);
      link = next;
    }
    head_ = tail_ = nullptr;
    count_ = 0;
  }

 private:
  using base_list::offset_;
  using base_list::head_;
  using base_list::tail_;
  using base_list::count_;
};

// Specialized IntrusiveListBase for std::unique_ptr types.
// This makes the list methods accept std::unique_ptrs and contains a special
// take() method that takes ownership of a list item.
template <typename T, size_t kOffset>
class IntrusiveListUniquePtrBase
    : private IntrusiveListBase<T, IntrusiveListIterator<T>, kOffset> {
 public:
  using iterator_type = IntrusiveListIterator<T>;
  using base_list = IntrusiveListBase<T, iterator_type, kOffset>;

  IntrusiveListUniquePtrBase() = default;

  using base_list::empty;
  using base_list::size;

  using base_list::contains;

  using base_list::clear;

  using base_list::begin;
  using base_list::end;
  using base_list::rbegin;
  using base_list::rend;

  using base_list::next;

  using base_list::previous;

  using base_list::front;

  void push_front(std::unique_ptr<T> value) {
    base_list::push_front(value.release());
  }

  using base_list::pop_front;

  using base_list::back;

  void push_back(std::unique_ptr<T> value) {
    base_list::push_back(value.release());
  }

  using base_list::pop_back;

  void insert(const iterator_type& it, std::unique_ptr<T> value) {
    base_list::insert(it, value.release());
  }

  using base_list::erase;

  // Removes an item from the list at the given iterator and transfers ownership
  // to the caller.
  // Performance: O(1)
  std::unique_ptr<T> take(iterator_type& it) {  // NOLINT(runtime/references)
    return take(*it);
  }

  // Removes an item from the list and transfers ownership to the caller.
  // Performance: O(1)
  std::unique_ptr<T> take(T* value) {
    if (!value) {
      return {nullptr};
    }
    auto link = impl::TToLink(offset_, value);
    if (link->prev) {
      DCHECK_NE(link, head_);
      link->prev->next = link->next;
    } else {
      DCHECK_EQ(link, head_);
      head_ = link->next;
    }
    if (link->next) {
      DCHECK_NE(link, tail_);
      link->next->prev = link->prev;
    } else {
      DCHECK_EQ(link, tail_);
      tail_ = link->prev;
    }
    link->next = link->prev = nullptr;
    --count_;
    base_list::OnRemove(value);
    base_list::CheckCorrectness();
    return std::unique_ptr<T>(value);
  }

  void replace(T* old_value, std::unique_ptr<T> new_value) {
    base_list::replace(old_value, new_value.release());
  }

  using base_list::sort;

 private:
  void OnDeallocate(T* value) override { delete value; }

  using base_list::offset_;
  using base_list::head_;
  using base_list::tail_;
  using base_list::count_;
};

// Specialized IntrusiveListBase for ref_ptr types.
// This makes the list methods accept/return ref_ptrs and iterate with
// a ref_ptr iterator.
template <typename T, size_t kOffset>
class IntrusiveListRefPtrBase
    : private IntrusiveListBase<T, IntrusiveListRefPtrIterator<T>, kOffset> {
 public:
  using iterator_type = IntrusiveListRefPtrIterator<T>;
  using base_list = IntrusiveListBase<T, iterator_type, kOffset>;

  IntrusiveListRefPtrBase() = default;

  using base_list::empty;
  using base_list::size;

  using base_list::contains;
  bool contains(const ref_ptr<T>& value) const {
    return base_list::contains(value.get());
  }

  using base_list::clear;

  using base_list::begin;
  using base_list::end;
  using base_list::rbegin;
  using base_list::rend;

  inline ref_ptr<T> next(const ref_ptr<T>& value) const {
    return ref_ptr<T>(base_list::next(value.get()));
  }
  inline ref_ptr<T> next(T* value) const {
    return ref_ptr<T>(base_list::next(value));
  }

  inline ref_ptr<T> previous(const ref_ptr<T>& value) const {
    return ref_ptr<T>(base_list::previous(value.get()));
  }
  inline ref_ptr<T> previous(T* value) const {
    return ref_ptr<T>(base_list::previous(value));
  }

  // Performance: O(1)
  inline ref_ptr<T> front() const {
    return ref_ptr<T>(impl::LinkToT<T>(offset_, head_));
  }

  void push_front(ref_ptr<T> value) { base_list::push_front(value.get()); }

  using base_list::pop_front;

  // Performance: O(1)
  inline ref_ptr<T> back() const {
    return ref_ptr<T>(impl::LinkToT<T>(offset_, tail_));
  }

  void push_back(ref_ptr<T> value) { base_list::push_back(value.get()); }

  using base_list::pop_back;

  void insert(const iterator_type& it, ref_ptr<T> value) {
    base_list::insert(it, value.get());
  }

  using base_list::erase;

  iterator_type erase(ref_ptr<T> value) {
    return base_list::erase(value.get());
  }

  void replace(const ref_ptr<T>& old_value, ref_ptr<T> new_value) {
    base_list::replace(old_value.get(), new_value.get());
  }
  void replace(T* old_value, ref_ptr<T> new_value) {
    base_list::replace(old_value, new_value.get());
  }

  using base_list::sort;

 private:
  void OnAdd(T* value) override { value->AddReference(); }
  void OnRemove(T* value) override { value->ReleaseReference(); }
  void OnDeallocate(T* value) override { value->ReleaseReference(); }

  using base_list::offset_;
  using base_list::head_;
  using base_list::tail_;
  using base_list::count_;
};

template <typename T, size_t kOffset = 0xffffffff>
class IntrusiveList : public IntrusiveListUnrefBase<T, kOffset> {};

template <typename T>
class IntrusiveList<T, 0xffffffff>
    : public IntrusiveListUnrefBase<T, offsetof(T, link)> {};

template <typename U, size_t kOffset>
class IntrusiveList<std::unique_ptr<U>, kOffset>
    : public IntrusiveListUniquePtrBase<U, kOffset> {};

template <typename U>
class IntrusiveList<std::unique_ptr<U>, 0xffffffff>
    : public IntrusiveListUniquePtrBase<U, offsetof(U, link)> {};

template <typename U, size_t kOffset>
class IntrusiveList<ref_ptr<U>, kOffset>
    : public IntrusiveListRefPtrBase<U, kOffset> {};

template <typename U>
class IntrusiveList<ref_ptr<U>, 0xffffffff>
    : public IntrusiveListRefPtrBase<U, offsetof(U, link)> {};

template <class T>
struct IntrusiveLinkBase : public T {
 public:
  IntrusiveListLink link;
};

template <>
struct IntrusiveLinkBase<void> {
 public:
  IntrusiveListLink link;
};

}  // namespace xrtl

#endif  // XRTL_BASE_INTRUSIVE_LIST_H_
