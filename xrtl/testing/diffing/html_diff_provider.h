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

#ifndef XRTL_TESTING_DIFFING_HTML_DIFF_PROVIDER_H_
#define XRTL_TESTING_DIFFING_HTML_DIFF_PROVIDER_H_

#include "xrtl/testing/diffing/diff_provider.h"

namespace xrtl {
namespace testing {
namespace diffing {

// Diff provider implementation that produces HTML outputs along with useful
// terminal logging.
class HtmlDiffProvider : public DiffProvider {
 public:
  HtmlDiffProvider();
  ~HtmlDiffProvider() override;

  bool Initialize(absl::string_view golden_base_path) override;

 protected:
  DiffResult PublishTextResult(DiffPublishMode publish_mode,
                               absl::string_view test_key,
                               absl::string_view text_value,
                               TextDiffer::Result compare_result,
                               DiffResult diff_result) override;
  DiffResult PublishDataResult(DiffPublishMode publish_mode,
                               absl::string_view test_key, const void* data,
                               size_t data_length,
                               DataDiffer::Result compare_result,
                               DiffResult diff_result) override;
  DiffResult PublishImageResult(DiffPublishMode publish_mode,
                                absl::string_view test_key,
                                ImageBuffer* image_buffer,
                                ImageDiffer::Result compare_result,
                                DiffResult diff_result) override;
};

}  // namespace diffing
}  // namespace testing
}  // namespace xrtl

#endif  // XRTL_TESTING_DIFFING_HTML_DIFF_PROVIDER_H_
