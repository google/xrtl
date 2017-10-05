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

#include <string>

#include "absl/strings/str_join.h"
#include "xrtl/base/logging.h"
#include "xrtl/testing/file_util.h"

namespace xrtl {
namespace testing {
namespace diffing {

HtmlDiffProvider::HtmlDiffProvider() = default;

HtmlDiffProvider::~HtmlDiffProvider() = default;

bool HtmlDiffProvider::Initialize(absl::string_view golden_base_path) {
  if (!DiffProvider::Initialize(golden_base_path)) {
    return false;
  }

  // TODO(benvanik): write file headers/open files for append/etc.

  return true;
}

DiffResult HtmlDiffProvider::PublishTextResult(
    DiffPublishMode publish_mode, absl::string_view test_key,
    absl::string_view text_value, TextDiffer::Result compare_result,
    DiffResult diff_result) {
  // Determine if we should be publishing the result.
  if (!CheckIfPublishRequired(publish_mode, diff_result)) {
    return diff_result;
  }

  std::string publish_file_path = FileUtil::MakeOutputFilePath(
      ResolveGoldenOutputFilePath(test_key, ".txt"));
  if (!FileUtil::SaveTextFile(publish_file_path, text_value)) {
    LOG(ERROR) << "Failed to save output when publishing result to "
               << publish_file_path;
    return DiffResult::kError;
  }

  LOG(INFO) << "$ cp " << publish_file_path << " "
            << MakeGoldenFilePath(test_key, ".txt");

  return diff_result;
}

DiffResult HtmlDiffProvider::PublishDataResult(
    DiffPublishMode publish_mode, absl::string_view test_key, const void* data,
    size_t data_length, DataDiffer::Result compare_result,
    DiffResult diff_result) {
  // Determine if we should be publishing the result.
  if (!CheckIfPublishRequired(publish_mode, diff_result)) {
    return diff_result;
  }

  if (data) {
    std::string publish_file_path = FileUtil::MakeOutputFilePath(
        ResolveGoldenOutputFilePath(test_key, ".bin"));
    if (!FileUtil::SaveFile(publish_file_path, data, data_length)) {
      LOG(ERROR) << "Failed to save output when publishing result to "
                 << publish_file_path;
      return DiffResult::kError;
    }

    LOG(INFO) << "$ cp " << publish_file_path << " "
              << MakeGoldenFilePath(test_key, ".bin");
  }

  return diff_result;
}

DiffResult HtmlDiffProvider::PublishImageResult(
    DiffPublishMode publish_mode, absl::string_view test_key,
    ImageBuffer* image_buffer, ImageDiffer::Result compare_result,
    DiffResult diff_result) {
  // Determine if we should be publishing the result.
  if (!CheckIfPublishRequired(publish_mode, diff_result)) {
    return diff_result;
  }

  if (image_buffer) {
    std::string publish_file_path = FileUtil::MakeOutputFilePath(
        ResolveGoldenOutputFilePath(test_key, ".png"));
    if (!image_buffer->Save(publish_file_path)) {
      LOG(ERROR) << "Failed to save output when publishing result to "
                 << publish_file_path;
      return DiffResult::kError;
    }

    LOG(INFO) << "$ cp " << publish_file_path << " "
              << MakeGoldenFilePath(test_key, ".png");
  }

  return diff_result;
}

}  // namespace diffing
}  // namespace testing
}  // namespace xrtl
