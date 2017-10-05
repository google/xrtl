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

#ifndef XRTL_TESTING_DIFFING_DIFF_PROVIDER_H_
#define XRTL_TESTING_DIFFING_DIFF_PROVIDER_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"
#include "xrtl/testing/diffing/data_differ.h"
#include "xrtl/testing/diffing/image_differ.h"
#include "xrtl/testing/diffing/text_differ.h"
#include "xrtl/testing/image_buffer.h"

namespace xrtl {
namespace testing {
namespace diffing {

// Controls how diff changes are published after a test comparison.
// How the files are published (such as submitted to a diff service, written to
// the local filesystem, etc) is up to implementations of DiffProvider.
enum class DiffPublishMode {
  // Always publishes changes.
  kAlways,
  // Never publishes changes.
  kNever,
  // Publishes only if the values differ or the reference is missing.
  kFailure,
};

// Defines the result of a value comparison operation.
// TODO(benvanik): extend to a struct with interesting flags/reporting values.
enum class DiffResult {
  // Both values are equivalent as defined by the comparison options.
  kEquivalent = 0,
  // The actual value differs from the expected value.
  kDifferent,
  // The expected reference value was not found.
  kMissingReference,
  // An error occurred during comparison.
  kError,
};

// Abstract diff provider interface.
// Providers may target different output formats, comparison services, or enable
// additional reporting.
//
// Usage:
//   auto diff_provider = DiffProvider::Create();
//   EXPECT_EQ(DiffResult::kEquivalent,
//             diff_provider->CompareText("my_test_key", "hello world!",
//                                        DiffPublishMode::kFailure, {}));
class DiffProvider {
 public:
  // Creates a diff provider based on the available set of providers and command
  // line flags.
  static std::unique_ptr<DiffProvider> Create();

  DiffProvider();
  virtual ~DiffProvider();

  // Runfiles-relative path to the golden data the diff provider will use when
  // loading keyed files.
  const std::string& golden_base_path() const { return golden_base_path_; }

  // Initializes the diff provider.
  // This may be expensive (spinning up RPC channels/etc) so it should only be
  // called once per test suite and reused for the entire test.
  // Returns true if the provider initialized and is ready for use.
  virtual bool Initialize(absl::string_view golden_base_path);

  // TODO(benvanik): CompareProto.

  // Compares a UTF8 text buffer with the data stored in the specified golden.
  // Returns the result of the comparison based on the provided options.
  virtual DiffResult CompareText(absl::string_view test_key,
                                 absl::string_view text_value,
                                 DiffPublishMode publish_mode,
                                 TextDiffer::Options options);

  // Compares a data buffer with the binary data stored in the specified golden.
  // Returns the result of the comparison based on the provided options.
  virtual DiffResult CompareData(absl::string_view test_key, const void* data,
                                 size_t data_length,
                                 DiffPublishMode publish_mode,
                                 DataDiffer::Options options);
  DiffResult CompareData(absl::string_view test_key,
                         const std::vector<uint8_t>& data,
                         DiffPublishMode publish_mode,
                         DataDiffer::Options options) {
    return CompareData(test_key, data.data(), data.size(), publish_mode,
                       std::move(options));
  }

  // Compares an image with the one stored in the specified golden.
  // The image is represented as a PNG-compressed byte buffer.
  // Returns the result of the comparison based on the provided options.
  virtual DiffResult CompareImage(absl::string_view test_key,
                                  ImageBuffer* image_buffer,
                                  DiffPublishMode publish_mode,
                                  ImageDiffer::Options options);

 protected:
  // Returns true if the result should be published based on the requested mode
  // and whether the test passed.
  bool CheckIfPublishRequired(DiffPublishMode publish_mode,
                              DiffResult diff_result);

  // Makes a test golden path for the given test key.
  // The file path must be resolved before attempting to load from it.
  std::string MakeGoldenFilePath(absl::string_view test_key,
                                 absl::string_view suffix = "");

  // Resolves an output test golden path to an absolute file path.
  // Updated goldens can be written here to be included in test outputs.
  std::string ResolveGoldenOutputFilePath(absl::string_view test_key,
                                          absl::string_view suffix = "");

  // TODO(benvanik): logging helpers (for binary diff, etc).

  // Publishes the results of a text diff based on the requested publish mode.
  virtual DiffResult PublishTextResult(DiffPublishMode publish_mode,
                                       absl::string_view test_key,
                                       absl::string_view text_value,
                                       TextDiffer::Result compare_result,
                                       DiffResult diff_result) {
    return diff_result;
  }

  // Publishes the results of a data diff based on the requested publish mode.
  virtual DiffResult PublishDataResult(DiffPublishMode publish_mode,
                                       absl::string_view test_key,
                                       const void* data, size_t data_length,
                                       DataDiffer::Result compare_result,
                                       DiffResult diff_result) {
    return diff_result;
  }

  // Publishes the results of an image diff based on the requested publish mode.
  virtual DiffResult PublishImageResult(DiffPublishMode publish_mode,
                                        absl::string_view test_key,
                                        ImageBuffer* image_buffer,
                                        ImageDiffer::Result compare_result,
                                        DiffResult diff_result) {
    return diff_result;
  }

 private:
  std::string golden_base_path_;
};

}  // namespace diffing
}  // namespace testing
}  // namespace xrtl

#endif  // XRTL_TESTING_DIFFING_DIFF_PROVIDER_H_
