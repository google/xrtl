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

#ifndef XRTL_BASE_MACROS_H_
#define XRTL_BASE_MACROS_H_

#include <array>
#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>

// Pulls in the XRTL_CONFIG_* defines.
#include "xrtl/tools/target_config/target_config.h"  // IWYU pragma: export
// Pulls in the XRTL_PLATFORM_* and XRTL_ARCH_* defines.
#include "xrtl/tools/target_platform/target_platform.h"  // IWYU pragma: export

// Debugging macros pull in leak checking/sanitization/etc based on config.
#include "xrtl/base/debugging_settings.h"  // IWYU pragma: export

// Bring in absl macros so we aren't double-including a macros.h everywhere.
#include "absl/base/attributes.h"
#include "absl/base/casts.h"
#include "absl/base/macros.h"  // IWYU pragma: export

// make_unique (until C++14 is everywhere).
#include "absl/memory/memory.h"  // IWYU pragma: export

#if defined(XRTL_COMPILER_GCC_COMPAT)

#define XRTL_EMPTY_FILE() static int dummy __attribute__((unused, used)) = 0;
#define XRTL_UNREACHABLE_DEFAULT()

#elif defined(XRTL_COMPILER_MSVC)

#define XRTL_EMPTY_FILE()
#define XRTL_UNREACHABLE_DEFAULT() \
  default:                         \
    DCHECK(false);

#else

#define XRTL_EMPTY_FILE()
#define XRTL_UNREACHABLE_DEFAULT() \
  default:                         \
    DCHECK(false);

#endif  // XRTL_COMPILER_[GCC_COMPAT/MSVC/etc]

// We're redefining this here, because pulling base/macros ends up pulling a
// ton of stuff that prevents us from bare metal compiling.
#define XRTL_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;           \
  void operator=(const TypeName&) = delete

// Macro from //base/allow_rvalue_references.h to disable the rvalue ref
// warning.
#if defined(GOOGLE_GLIBCXX_VERSION) && defined(__clang__) && !defined(__APPLE__)
#define XRTL_ALLOW_RVALUE_REFERENCES_PUSH \
  _Pragma("GCC diagnostic push")          \
      _Pragma("GCC diagnostic ignored \"-Wgoogle3-rvalue-reference\"")
#define XRTL_ALLOW_RVALUE_REFERENCES_POP _Pragma("GCC diagnostic pop")
#else  // other compiler case follows
#define XRTL_ALLOW_RVALUE_REFERENCES_PUSH
#define XRTL_ALLOW_RVALUE_REFERENCES_POP
#endif  // other compiler case

namespace xrtl {

// Makes an std::array from a list of arguments.
//
// Usage:
//  std::array<int, 3> value = make_array(1, 2, 3);
//  (or, more commonly)
//  DoSomething(make_array(1, 2, 3));
template <typename... T>
constexpr auto make_array(T&&... values) -> std::array<
    typename std::decay<typename std::common_type<T...>::type>::type,
    sizeof...(T)> {
  return std::array<
      typename std::decay<typename std::common_type<T...>::type>::type,
      sizeof...(T)>{{std::forward<T>(values)...}};
}

// Utility to enable bitmask operators on enum classes.
// To use define an enum class with valid bitmask values and an underlying type
// then use the macro to enable support:
//  enum class MyBitmask : uint32_t {
//    kFoo = 1 << 0,
//    kBar = 1 << 1,
//  };
//  XRTL_BITMASK(MyBitmask);
//  MyBitmask value = ~(MyBitmask::kFoo | MyBitmask::kBar);
#define XRTL_BITMASK(enum_class)                                       \
  inline enum_class operator|(enum_class lhs, enum_class rhs) {        \
    typedef typename std::underlying_type<enum_class>::type enum_type; \
    return static_cast<enum_class>(static_cast<enum_type>(lhs) |       \
                                   static_cast<enum_type>(rhs));       \
  }                                                                    \
  inline enum_class& operator|=(enum_class& lhs, enum_class rhs) {     \
    typedef typename std::underlying_type<enum_class>::type enum_type; \
    lhs = static_cast<enum_class>(static_cast<enum_type>(lhs) |        \
                                  static_cast<enum_type>(rhs));        \
    return lhs;                                                        \
  }                                                                    \
  inline enum_class operator&(enum_class lhs, enum_class rhs) {        \
    typedef typename std::underlying_type<enum_class>::type enum_type; \
    return static_cast<enum_class>(static_cast<enum_type>(lhs) &       \
                                   static_cast<enum_type>(rhs));       \
  }                                                                    \
  inline enum_class& operator&=(enum_class& lhs, enum_class rhs) {     \
    typedef typename std::underlying_type<enum_class>::type enum_type; \
    lhs = static_cast<enum_class>(static_cast<enum_type>(lhs) &        \
                                  static_cast<enum_type>(rhs));        \
    return lhs;                                                        \
  }                                                                    \
  inline enum_class operator^(enum_class lhs, enum_class rhs) {        \
    typedef typename std::underlying_type<enum_class>::type enum_type; \
    return static_cast<enum_class>(static_cast<enum_type>(lhs) ^       \
                                   static_cast<enum_type>(rhs));       \
  }                                                                    \
  inline enum_class& operator^=(enum_class& lhs, enum_class rhs) {     \
    typedef typename std::underlying_type<enum_class>::type enum_type; \
    lhs = static_cast<enum_class>(static_cast<enum_type>(lhs) ^        \
                                  static_cast<enum_type>(rhs));        \
    return lhs;                                                        \
  }                                                                    \
  inline enum_class operator~(enum_class lhs) {                        \
    typedef typename std::underlying_type<enum_class>::type enum_type; \
    return static_cast<enum_class>(~static_cast<enum_type>(lhs));      \
  }                                                                    \
  inline bool any(enum_class lhs) {                                    \
    typedef typename std::underlying_type<enum_class>::type enum_type; \
    return static_cast<enum_type>(lhs) != 0;                           \
  }

// A helper wrapper that moves the wrapped object on copy.
// This is particularly handy for capturing unique_ptrs in lambdas.
//
// Usage example:
//  std::unique_ptr<Foo> foo_ptr(new Foo());
//  move_on_copy<std::unique_ptr<Foo>> foo(std::move(foo_ptr));
//  auto some_lambda = [bar]() { ... }
//
template <typename T>
struct move_on_copy {
  explicit move_on_copy(T&& t) : value(std::move(t)) {}
  move_on_copy(move_on_copy const& other) : value(std::move(other.value)) {}
  move_on_copy(move_on_copy&& other) : value(std::move(other.value)) {}
  move_on_copy& operator=(move_on_copy const& other) {
    value = std::move(other.value);
    return *this;
  }
  move_on_copy& operator=(move_on_copy&& other) {
    value = std::move(other.value);
    return *this;
  }
  mutable T value;
};

// Utility to aid in moving ref_ptr's into closures.
//
// Usage:
//  auto baton = MoveToLambda(my_ref);
//  DoSomething([baton] () { baton.value; });
#define MoveToLambda(p) xrtl::move_on_copy<decltype(p)>(std::move(p))

}  // namespace xrtl

#endif  // XRTL_BASE_MACROS_H_
