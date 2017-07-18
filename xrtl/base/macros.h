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

// Temporary make_unique (until C++14 is everywhere).
#include "xrtl/base/make_unique.h"  // IWYU pragma: export

#if defined(XRTL_COMPILER_GCC_COMPAT)

#define XRTL_ALWAYS_INLINE __attribute__((always_inline)) inline
#define XRTL_ATTRIBUTE_NORETURN __attribute__((noreturn))
#define XRTL_ATTRIBUTE_NOINLINE __attribute__((noinline))
#define XRTL_ATTRIBUTE_UNUSED __attribute__((unused))
#define XRTL_ATTRIBUTE_COLD __attribute__((cold))
#define XRTL_ATTRIBUTE_WEAK __attribute__((weak))
#define XRTL_ATTRIBUTE_NOSANITIZE __attribute__((no_sanitize_address))
#define XRTL_EMPTY_FILE() static int dummy __attribute__((unused, used)) = 0;
#define XRTL_UNREACHABLE_DEFAULT()

#elif defined(XRTL_COMPILER_MSVC)

#define XRTL_ALWAYS_INLINE __forceinline
#define XRTL_ATTRIBUTE_NORETURN __declspec(noreturn)
#define XRTL_ATTRIBUTE_NOINLINE
#define XRTL_ATTRIBUTE_UNUSED
#define XRTL_ATTRIBUTE_COLD
#define XRTL_ATTRIBUTE_WEAK
#define XRTL_ATTRIBUTE_NOSANITIZE
#define XRTL_EMPTY_FILE()
#define XRTL_UNREACHABLE_DEFAULT() \
  default:                         \
    DCHECK(false);

#else

#define XRTL_ALWAYS_INLINE inline
#define XRTL_ATTRIBUTE_NORETURN
#define XRTL_ATTRIBUTE_NOINLINE
#define XRTL_ATTRIBUTE_UNUSED
#define XRTL_ATTRIBUTE_COLD
#define XRTL_ATTRIBUTE_WEAK
#define XRTL_ATTRIBUTE_NOSANITIZE
#define XRTL_EMPTY_FILE()
#define XRTL_UNREACHABLE_DEFAULT() \
  default:                         \
    DCHECK(false);

#endif  // XRTL_COMPILER_[GCC_COMPAT/MSVC/etc]

#define XRTL_PREDICT_FALSE(x) (x)
#define XRTL_PREDICT_TRUE(x) (x)

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
// Define this to 1 if the code is compiled in C++11 mode.
#define LANG_CXX11 1
#endif

#if defined(__clang__) && defined(LANG_CXX11) && defined(__has_warning)
#if __has_feature(cxx_attributes) && __has_warning("-Wimplicit-fallthrough")
#define XRTL_FALLTHROUGH_INTENDED [[clang::fallthrough]]  // NOLINT
#endif  // -Wimplicit-fallthrough
#endif  // clang && c++11+
#ifndef XRTL_FALLTHROUGH_INTENDED
#define XRTL_FALLTHROUGH_INTENDED \
  do {                            \
  } while (0)
#endif  // !XRTL_FALLTHROUGH_INTENDED

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

// Type-safe countof for determining constant array length.
template <typename T, std::size_t N>
inline constexpr std::size_t count_of(T const (&)[N]) noexcept {
  return N;
}

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

// Casts the bits of one type to another of equal size without conversion.
// Example:
//   float f = 3.14159265358979;
//   int i = bit_cast<int32_t>(f);
//   // i = 0x40490fdb
template <class Dest, class Source>
inline Dest bit_cast(const Source& source) {
  static_assert(sizeof(Dest) == sizeof(Source), "Type sizes must match");
  Dest dest;
  std::memcpy(&dest, &source, sizeof(dest));
  return dest;
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
