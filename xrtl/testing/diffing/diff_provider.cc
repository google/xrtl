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

#include "xrtl/testing/diffing/diff_provider.h"

#include "absl/strings/str_join.h"
#include "xrtl/base/logging.h"
#include "xrtl/testing/file_util.h"

namespace xrtl {
namespace testing {
namespace diffing {

DiffProvider::DiffProvider() = default;

DiffProvider::~DiffProvider() = default;

bool DiffProvider::Initialize(absl::string_view golden_base_path) {
  golden_base_path_ = std::string(golden_base_path);
  return true;
}

bool DiffProvider::CheckIfPublishRequired(DiffPublishMode publish_mode,
                                          DiffResult diff_result) {
  // Determine if we should be publishing the result.
  bool did_pass = diff_result == DiffResult::kEquivalent;
  bool needs_publish = false;
  switch (publish_mode) {
    case DiffPublishMode::kAlways:
      VLOG(1) << "Forcing publish because publish mode is kAlways";
      needs_publish = true;
      break;
    case DiffPublishMode::kNever:
      VLOG(1) << "Skipping publish because publish mode is kNever";
      needs_publish = false;
      break;
    case DiffPublishMode::kFailure:
      needs_publish = !did_pass;
      break;
  }
  return needs_publish;
}

std::string DiffProvider::MakeGoldenFilePath(absl::string_view test_key,
                                             absl::string_view suffix) {
  std::string relative_path = FileUtil::JoinPathParts(
      golden_base_path_, absl::StrJoin({test_key, suffix}, ""));
  return relative_path;
}

std::string DiffProvider::ResolveGoldenOutputFilePath(
    absl::string_view test_key, absl::string_view suffix) {
  std::string relative_path = MakeGoldenFilePath(test_key, suffix);
  return FileUtil::MakeOutputFilePath(relative_path);
}

DiffResult DiffProvider::CompareText(absl::string_view test_key,
                                     absl::string_view text_value,
                                     DiffPublishMode publish_mode,
                                     TextDiffer::Options options) {
  // Load reference file.
  std::string golden_file_path = MakeGoldenFilePath(test_key, ".txt");
  auto golden_text_buffer = FileUtil::LoadTextFile(golden_file_path);
  if (!golden_text_buffer) {
    LOG(ERROR) << "Unable to find reference file at " << golden_file_path;
    return PublishTextResult(publish_mode, test_key, text_value, {},
                             DiffResult::kMissingReference);
  }

  // Diff the text.
  auto result = TextDiffer::DiffStrings(golden_text_buffer.value(), text_value,
                                        std::move(options));
  return PublishTextResult(
      publish_mode, test_key, text_value, result,
      result.equivalent ? DiffResult::kEquivalent : DiffResult::kDifferent);
}

DiffResult DiffProvider::CompareData(absl::string_view test_key,
                                     const void* data, size_t data_length,
                                     DiffPublishMode publish_mode,
                                     DataDiffer::Options options) {
  // Load reference file.
  std::string golden_file_path = MakeGoldenFilePath(test_key, ".bin");
  auto golden_data_buffer = FileUtil::LoadFile(golden_file_path);
  if (!golden_data_buffer) {
    LOG(ERROR) << "Unable to find reference file at " << golden_file_path;
    return PublishDataResult(publish_mode, test_key, data, data_length, {},
                             DiffResult::kMissingReference);
  }

  // Diff the data.
  auto result = DataDiffer::DiffBuffers(golden_data_buffer.value().data(),
                                        golden_data_buffer.value().size(), data,
                                        data_length, std::move(options));
  return PublishDataResult(
      publish_mode, test_key, data, data_length, result,
      result.equivalent ? DiffResult::kEquivalent : DiffResult::kDifferent);
}

DiffResult DiffProvider::CompareImage(absl::string_view test_key,
                                      ImageBuffer* image_buffer,
                                      DiffPublishMode publish_mode,
                                      ImageDiffer::Options options) {
  // Load reference image.
  std::string golden_file_path = MakeGoldenFilePath(test_key, ".png");
  auto golden_image_buffer =
      ImageBuffer::Load(golden_file_path, image_buffer->channels());
  if (!golden_image_buffer) {
    LOG(ERROR) << "Unable to find reference file at " << golden_file_path;
    return PublishImageResult(publish_mode, test_key, image_buffer, {},
                              DiffResult::kMissingReference);
  }

  // Diff the images.
  auto result = ImageDiffer::DiffImageBuffers(golden_image_buffer.get(),
                                              image_buffer, std::move(options));
  return PublishImageResult(
      publish_mode, test_key, image_buffer, result,
      result.equivalent ? DiffResult::kEquivalent : DiffResult::kDifferent);
}

}  // namespace diffing
}  // namespace testing
}  // namespace xrtl
