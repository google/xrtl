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

// This file contains simple geometry/dimension helpers used in common code.
// For hardcore math please use glm and the types under xrtl::math instead.

#ifndef XRTL_BASE_GEOMETRY_H_
#define XRTL_BASE_GEOMETRY_H_

namespace xrtl {

struct Point2D {
  int x = 0;
  int y = 0;

  Point2D() = default;
  Point2D(int x, int y) : x(x), y(y) {}

  bool operator==(const Point2D& other) const {
    return x == other.x && y == other.y;
  }
  bool operator!=(const Point2D& other) const { return !(*this == other); }
};

struct Size2D {
  int width = 0;
  int height = 0;

  Size2D() = default;
  Size2D(int width, int height) : width(width), height(height) {}

  bool operator==(const Size2D& other) const {
    return width == other.width && height == other.height;
  }
  bool operator!=(const Size2D& other) const { return !(*this == other); }
};

struct Rect2D {
  Point2D origin;
  Size2D size;

  Rect2D() = default;
  Rect2D(Point2D origin, Size2D size) : origin(origin), size(size) {}
  Rect2D(int x, int y, int width, int height)
      : origin(x, y), size(width, height) {}

  bool operator==(const Rect2D& other) const {
    return origin == other.origin && size == other.size;
  }
  bool operator!=(const Rect2D& other) const { return !(*this == other); }

  // Returns true if this Rect2D intersects with another Rect2D.
  bool TestIntersection(const Rect2D& other) {
    return !(other.origin.x > origin.x + size.width ||
             other.origin.x + other.size.width < origin.x ||
             other.origin.y > origin.y + size.height ||
             other.origin.y + other.size.height < origin.y);
  }
};

struct Frame2D {
  int left = 0;
  int top = 0;
  int right = 0;
  int bottom = 0;

  Frame2D() = default;
  Frame2D(int left, int top, int right, int bottom)
      : left(left), top(top), right(right), bottom(bottom) {}

  bool operator==(const Frame2D& other) const {
    return left == other.left && top == other.top && right == other.right &&
           bottom == other.bottom;
  }
  bool operator!=(const Frame2D& other) const { return !(*this == other); }
};

}  // namespace xrtl

#endif  // XRTL_BASE_GEOMETRY_H_
