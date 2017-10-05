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

#include "xrtl/testing/diffing/html_diff_provider.h"

#include "xrtl/base/macros.h"

namespace xrtl {
namespace testing {
namespace diffing {

// Factory function to create the HtmlDiffProvider as the default provider.
// Depend on //xrtl/testing/diffing:html_diff_provider_create to link this in.
std::unique_ptr<DiffProvider> DiffProvider::Create() {
  return absl::make_unique<HtmlDiffProvider>();
}

}  // namespace diffing
}  // namespace testing
}  // namespace xrtl
