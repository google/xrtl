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

#include "xrtl/testing/diffing/data_differ.h"

#include "xrtl/base/logging.h"

namespace xrtl {
namespace testing {
namespace diffing {

DataDiffer::Result DataDiffer::DiffBuffers(const void* expected_value,
                                           size_t expected_value_length,
                                           const void* actual_value,
                                           size_t actual_value_length,
                                           Options options) {
  Result result;
  result.equivalent = true;
  if (expected_value_length != actual_value_length) {
    LOG(ERROR) << "Expected data length " << expected_value_length
               << " but got actual " << actual_value_length;
    result.equivalent = false;
  } else {
    if (std::memcmp(expected_value, actual_value, expected_value_length) != 0) {
      LOG(ERROR) << "One or more data bytes differ";
      result.equivalent = false;
    }
  }
  return result;
}

}  // namespace diffing
}  // namespace testing
}  // namespace xrtl
