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

#ifndef XRTL_BASE_MATH_H_
#define XRTL_BASE_MATH_H_

#include <algorithm>
#include <cmath>

#include "xrtl/base/macros.h"

namespace xrtl {
namespace math {

// Returns true if the difference between two values is within epsilon.
ABSL_ATTRIBUTE_ALWAYS_INLINE constexpr bool AreAlmostEqual(double a, double b,
                                                           double epsilon) {
  return std::abs(a - b) <= epsilon;
}

// Returns -1 or 1 depending on the sign of the input.
template <typename T>
ABSL_ATTRIBUTE_ALWAYS_INLINE constexpr T Sign(T v) noexcept {
  return v >= 0 ? T(1) : T(-1);
}

// Returns the linear interpolation t percent between a and b.
//   a: Lower bound on interpolation range.
//   b: Upper bound on interpolation range.
//   t: Interpolation value [0, 1].
template <typename T, typename V>
ABSL_ATTRIBUTE_ALWAYS_INLINE constexpr V Lerp(T a, T b, V t) noexcept {
  return a + t * (b - a);
}

// Calculates the parameter t that produces x when linearly interpolating
// between a and b.
//
// Example:
//   a = 10, b = 20, x = 18
//   returns 0.8 since lerp(10, 20, 0.8) returns 18
//
//   a: Lower bound on interpolation range.
//   b: Upper bound on interpolation range.
//   x: Value between a and b to find t for.
//   Returns the interpolation value t [0, 1] for x between a and b.
template <typename T, typename V>
ABSL_ATTRIBUTE_ALWAYS_INLINE constexpr V InverseLerp(T a, T b, V x) noexcept {
  return (x - a) / (b - a);
}

// Constrains a value to the given range with scaling.
// This allows the value to go outside of the range with increasing tension to
// pull it back toward the bounds. This is useful for adding overdrag and
// velocity padding.
//
//   value: Value to constrain.
//   min_value: Minimum value before which the value will be constrained.
//   max_value: Maximum value after which the value will be constrained.
//   scale: Scale applied to the value when stretching. 1/2 and 1/3 are common.
template <typename T>
ABSL_ATTRIBUTE_ALWAYS_INLINE T Constrain(T value, T min_value, T max_value,
                                         T scale) noexcept {
  if (value < min_value) {
    return min_value + (value - min_value) * scale;
  } else if (value > max_value) {
    return max_value + (value - max_value) * scale;
  } else {
    return value;
  }
}

// Clamps a value between a min and max.
template <typename T>
ABSL_ATTRIBUTE_ALWAYS_INLINE T Clamp(T value, T min, T max) noexcept {
  return value <= min ? min : value >= max ? max : value;
}

// Wraps a signed number such that it is within [0,max).
// This matches an arithmetic % (not the stupid C++ one).
//    Wrap(0, 4) = 0
//    Wrap(4, 4) = 0
//    Wrap(5, 4) = 1
//   Wrap(-1, 4) = 3
ABSL_ATTRIBUTE_ALWAYS_INLINE constexpr int Wrap(int value, int max) noexcept {
  return (value % max + max) % max;
}

// Rounds up to the next alignment value, if it is not already aligned.
template <typename T>
ABSL_ATTRIBUTE_ALWAYS_INLINE constexpr T RoundToAlignment(
    T value, T alignment) noexcept {
  return ((value + alignment - 1) / alignment) * alignment;
}

// Rounds the value up to the next power of two, if not a power of two already.
template <typename T>
ABSL_ATTRIBUTE_ALWAYS_INLINE constexpr int RoundToNextPowerOfTwo(
    T value) noexcept {
  return static_cast<T>(std::pow(2, std::ceil(std::log(value) / std::log(2))));
}

}  // namespace math
}  // namespace xrtl

#endif  // XRTL_BASE_MATH_H_
