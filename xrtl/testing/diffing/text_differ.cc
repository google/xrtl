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

#include "xrtl/testing/diffing/text_differ.h"

#include "xrtl/base/logging.h"

namespace xrtl {
namespace testing {
namespace diffing {

TextDiffer::Result TextDiffer::DiffStrings(absl::string_view expected_value,
                                           absl::string_view actual_value,
                                           Options options) {
  Result result;
  result.equivalent = true;
  if (expected_value.size() != actual_value.size()) {
    LOG(ERROR) << "Expected string length " << expected_value.size()
               << " but got actual " << actual_value.size();
    result.equivalent = false;
  } else {
    if (std::memcmp(expected_value.data(), actual_value.data(),
                    expected_value.size()) != 0) {
      LOG(ERROR) << "One or more characters differ" << std::endl
                 << "Expected: " << expected_value << std::endl
                 << "Actual: " << actual_value;
      result.equivalent = false;
    }
  }
  return result;
}

}  // namespace diffing
}  // namespace testing
}  // namespace xrtl
