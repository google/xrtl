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

#ifndef XRTL_TESTING_DIFFING_TEXT_DIFFER_H_
#define XRTL_TESTING_DIFFING_TEXT_DIFFER_H_

#include <utility>

#include "absl/strings/string_view.h"

namespace xrtl {
namespace testing {
namespace diffing {

// Utilities for diffing human-readable text.
class TextDiffer {
 public:
  enum Mode {
    kDefault = 0,
  };

  // Options that can be used to adjust the text comparison operation.
  struct Options {
    // Comparison mode.
    Mode mode = Mode::kDefault;
  };

  // Result of a diffing operation.
  struct Result {
    // True if the expected and actual text strings were equivalent
    // as defined by the comparison mode and options.
    bool equivalent = false;

    // TODO(benvanik): diff text/etc.
  };

  // Diffs a string against its expected value and returns the result.
  static Result DiffStrings(absl::string_view expected_value,
                            absl::string_view actual_value, Options options);

  // Compares two text strings against each other.
  // Returns false if the test string does not match the expected string.
  static bool CompareStrings(absl::string_view expected_value,
                             absl::string_view actual_value, Options options) {
    return DiffStrings(expected_value, actual_value, std::move(options))
        .equivalent;
  }
};

}  // namespace diffing
}  // namespace testing
}  // namespace xrtl

#endif  // XRTL_TESTING_DIFFING_TEXT_DIFFER_H_
