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

#ifndef XRTL_TESTING_DIFFING_DATA_DIFFER_H_
#define XRTL_TESTING_DIFFING_DATA_DIFFER_H_

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace xrtl {
namespace testing {
namespace diffing {

// Utilities for diffing binary data buffers.
class DataDiffer {
 public:
  enum Mode {
    kDefault = 0,
  };

  // Options that can be used to adjust the data comparison operation.
  struct Options {
    // Comparison mode.
    Mode mode = Mode::kDefault;
  };

  // Result of a diffing operation.
  struct Result {
    // True if the expected and actual data buffers were equivalent
    // as defined by the comparison mode and options.
    bool equivalent = false;

    // TODO(benvanik): diff text/etc.
  };

  // Diffs a buffer against its expected value and returns the result.
  static Result DiffBuffers(const void* expected_value,
                            size_t expected_value_length,
                            const void* actual_value,
                            size_t actual_value_length, Options options);
  static Result DiffBuffers(const std::vector<uint8_t>& expected_value,
                            const std::vector<uint8_t>& actual_value,
                            Options options) {
    return DiffBuffers(expected_value.data(), expected_value.size(),
                       actual_value.data(), actual_value.size(),
                       std::move(options));
  }

  // Compares two buffers against each other.
  // Returns false if the test buffer does not match the expected buffer.
  static bool CompareBuffers(const void* expected_value,
                             size_t expected_value_length,
                             const void* actual_value,
                             size_t actual_value_length, Options options) {
    return DiffBuffers(expected_value, expected_value_length, actual_value,
                       actual_value_length, std::move(options))
        .equivalent;
  }
  static bool CompareBuffers(const std::vector<uint8_t>& expected_value,
                             const std::vector<uint8_t>& actual_value,
                             Options options) {
    return CompareBuffers(expected_value.data(), expected_value.size(),
                          actual_value.data(), actual_value.size(),
                          std::move(options));
  }
};

}  // namespace diffing
}  // namespace testing
}  // namespace xrtl

#endif  // XRTL_TESTING_DIFFING_DATA_DIFFER_H_
